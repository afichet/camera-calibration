#include "imagemodel.h"

#include <cstddef>
#include <cmath>
#include <array>
#include <fstream>

extern "C"
{
#include <image.h>
#include <color-converter.h>
#include <io.h>
}

#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFile>

ImageModel::ImageModel()
  : QObject()
  , _mosaicedPixelBuffer(nullptr)
  , _pixelBuffer(nullptr)
  , _correctionMatrix({1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f})
  , _isImageLoaded(false)
  , _isMatrixLoaded(false)
  , _isMatrixActive(false)
  , _isRawImage(false)
  , _innerMarginX(0.01)
  , _innerMarginY(0.01)
  , _exposure(0)
  , _demosaicingMethod(RAWDemosaicMethod::VNG4)
  , _filters(0x49494949)
  , _imageLoadingWatcher(new QFutureWatcher<void>(this))
  , _imageDemosaicingWatcher(new QFutureWatcher<void>(this))
  , _imageEditingWatcher(new QFutureWatcher<void>(this))
{
  _macbethOutline << QPointF(0, 0) << QPointF(100, 0) << QPointF(100, 100) << QPointF(0, 100);

  recalculateMacbethPatches();
}


ImageModel::~ImageModel()
{
  free(_mosaicedPixelBuffer);
  free(_pixelBuffer);
}


void ImageModel::getAveragedPatches(std::vector<float> &values)
{
  std::vector<float> patches_values(4 * _macbethPatches.size(), 0.f);

  for (int y = 0; y < _image.height(); y++)
  {
    #pragma omp parallel for
    for (int x = 0; x < _image.width(); x++)
    {
      const QPointF currentPixel(x, y);

      for (int p = 0; p < _macbethPatches.size(); p++)
      {
        if (_macbethPatches[p].containsPoint(currentPixel, Qt::OddEvenFill))
        {
          for (int c = 0; c < 3; c++)
          {
            patches_values[4 * p + c] += _pixelBuffer[3 * (y * _image.width() + x) + c];
          }
          patches_values[4 * p + 3] += 1;
          //break;
        }
      }
    }

    emit processProgress(int(100.f * float(y) / float(_image.height() - 1)));
  }

  values.resize(3 * _macbethPatches.size());

  #pragma omp parallel for
  for (int p = 0; p < _macbethPatches.size(); p++)
  {
    for (int c = 0; c < 3; c++)
    {
      values[3 * p + c] = patches_values[4 * p + c] / patches_values[4 * p + 3];
    }
  }
}


void ImageModel::openFile(const QString &filename)
{
  if (filename.endsWith(".tiff", Qt::CaseInsensitive) || filename.endsWith(".tif", Qt::CaseInsensitive)
      || filename.endsWith(".exr", Qt::CaseInsensitive) || filename.endsWith(".txt", Qt::CaseInsensitive))
  {
    openImage(filename);
  }
  else if (filename.endsWith(".csv", Qt::CaseInsensitive))
  {
    openCorrectionMatrix(filename);
  }
}


void ImageModel::openImage(const QString &filename)
{
  free(_mosaicedPixelBuffer);
  free(_pixelBuffer);

  _mosaicedPixelBuffer = nullptr;
  _pixelBuffer         = nullptr;
  _isImageLoaded       = false;
  _isRawImage          = false;

  if (_imageEditingWatcher->isRunning())
  {
    emit _imageEditingWatcher->cancel();
    _imageEditingWatcher->waitForFinished();
  }

  if (_imageLoadingWatcher->isRunning())
  {
    emit _imageLoadingWatcher->cancel();
    _imageLoadingWatcher->waitForFinished();
  }

  QFuture<void> imageLoading = QtConcurrent::run([=]() {
    emit   processProgress(0);
    emit   loadingMessage(tr("Loading image..."));
    size_t width, height;

    int success = 0;

    if (filename.endsWith(".txt", Qt::CaseInsensitive))
    {
      // We are opening a RAW file
      success = read_raw_file(filename.toStdString().c_str(), &_mosaicedPixelBuffer, &width, &height, &_filters);
      if (success != 0)
      {
        emit loadFailed(tr("Cannot open image file"));
        emit loadingMessage("");
        delete[] _pixelBuffer;
        _pixelBuffer = nullptr;
        return;
      }
      _isRawImage  = true;
      _pixelBuffer = (float *)calloc(3 * width * height, sizeof(float));
      demosaic(_mosaicedPixelBuffer, _pixelBuffer, width, height, _filters, _demosaicingMethod);
    }
    else
    {
      success = read_image(filename.toStdString().c_str(), &_pixelBuffer, &width, &height);

      if (success != 0)
      {
        emit loadFailed(tr("Cannot open image file"));
        emit loadingMessage("");
        delete[] _pixelBuffer;
        _pixelBuffer = nullptr;
        return;
      }
    }

    _exposure = 0.f;
    _pixelCorrected.resize(3 * width * height);
    _image         = QImage(width, height, QImage::Format_RGB888);
    _isImageLoaded = true;
    _imagePath     = filename;
    _macbethOutline.clear();
    _macbethOutline << QPointF(0, 0) << QPointF(_image.width(), 0) << QPointF(_image.width(), _image.height())
                    << QPointF(0, _image.height());

    recalculateMacbethPatches();

    emit processProgress(100);
    emit loadingMessage("");
    emit exposureChanged(_exposure);
    emit imageLoaded(width, height);
  });

  _imageLoadingWatcher->setFuture(imageLoading);
  recalculateCorrection(0);
}


void ImageModel::openCorrectionMatrix(const QString &filename)
{
  QFile f(filename);

  f.open(QFile::ReadOnly | QFile::Text);

  QTextStream ts(&f);

  std::array<float, 9> tempMatrix;
  int                  lines = 0;

  while (!ts.atEnd() || lines < 2)
  {
    QString     line = ts.readLine();
    QStringList vals = line.split(",");
    if (vals.size() < 3)
    {
      return;
    }

    for (int v = 0; v < 3; v++)
    {
      tempMatrix[3 * lines + v] = vals[v].toFloat();
    }

    ++lines;
  }

  if (lines < 3)
  {
    return;
  }

  _isMatrixActive = true;
  emit matrixActivationStateChanged(_isMatrixActive);

  setMatrix(tempMatrix);
}


void ImageModel::setInnerMarginX(float position)
{
  _innerMarginX = position;
  recalculateMacbethPatches();
}


void ImageModel::setInnerMarginY(float position)
{
  _innerMarginY = position;
  recalculateMacbethPatches();
}


void ImageModel::setOutlinePosition(int index, QPointF position)
{
  _macbethOutline[index] = position;
  recalculateMacbethPatches();
}


void ImageModel::setExposure(double value)
{
  recalculateCorrection(value);
}


void ImageModel::setDemosaicingMethod(const QString &method)
{
  if (_imageLoadingWatcher->isRunning())
  {
    _imageLoadingWatcher->waitForFinished();
  }

  if (!isImageLoaded()) return;

  if (_imageDemosaicingWatcher->isRunning())
  {
    _imageDemosaicingWatcher->waitForFinished();
  }

  if (_imageEditingWatcher->isRunning())
  {
    _imageEditingWatcher->waitForFinished();
  }

  if (method == "None")
  {
    _demosaicingMethod = RAWDemosaicMethod::NONE;
  }
  else if (method == "Basic")
  {
    _demosaicingMethod = RAWDemosaicMethod::BASIC;
  }
  else if (method == "VNG4")
  {
    _demosaicingMethod = RAWDemosaicMethod::VNG4;
  }
  else if (method == "AHD")
  {
    _demosaicingMethod = RAWDemosaicMethod::AHD;
  }
  else if (method == "RCD")
  {
    _demosaicingMethod = RAWDemosaicMethod::RCD;
  }
  else if (method == "AMaZE")
  {
    _demosaicingMethod = RAWDemosaicMethod::AMAZE;
  }

  if (_imageEditingWatcher->isRunning())
  {
    _imageEditingWatcher->waitForFinished();
  }

  if (_isRawImage)
  {
    QFuture<void> demosaicing = QtConcurrent::run([=]() {
      demosaic(_mosaicedPixelBuffer, _pixelBuffer, _image.width(), _image.height(), _filters, _demosaicingMethod);
    });

    _imageDemosaicingWatcher->setFuture(demosaicing);
    recalculateCorrection(_exposure);
  }
}


void ImageModel::setMatrix(const std::array<float, 9> matrix)
{
  if (_correctionMatrix == matrix) return;

  _isMatrixLoaded   = true;
  _correctionMatrix = matrix;

  if (_isMatrixActive) recalculateCorrection(_exposure);
  emit matrixLoaded(_correctionMatrix);
  setMatrixActive(true);
}


void ImageModel::setMatrixActive(bool active)
{
  if (_isMatrixActive == active) return;

  _isMatrixActive = active;
  recalculateCorrection(_exposure);
  emit matrixActivationStateChanged(_isMatrixActive);
}


void ImageModel::savePatchesCoordinates(const QString &filename)
{
  if (!isImageLoaded()) return;

  if (_imageLoadingWatcher->isRunning())
  {
    _imageLoadingWatcher->waitForFinished();
  }

  QFuture<void> imageLoading = QtConcurrent::run([=]() {
    emit processProgress(0);
    emit loadingMessage(tr("Saving patches coordinates..."));

    std::ofstream outputFile(filename.toStdString());

    for (const QPolygonF &patch : _macbethPatches)
    {
      for (const QPointF &p : patch)
      {
        outputFile << p.x() << ", " << p.y() << "; ";
      }
      outputFile << std::endl;
    }

    emit processProgress(100);
    emit loadingMessage("");
  });

  _imageLoadingWatcher->setFuture(imageLoading);
}


void ImageModel::savePatchesColors(const QString &filename)
{
  if (_imageLoadingWatcher->isRunning())
  {
    _imageLoadingWatcher->waitForFinished();
  }

  if (!isImageLoaded()) return;

  QFuture<void> imageLoading = QtConcurrent::run([=]() {
    emit processProgress(0);
    emit loadingMessage(tr("Saving patches colors..."));

    std::vector<float> final_values;

    getAveragedPatches(final_values);

    std::ofstream outputFile(filename.toStdString());

    for (int p = 0; p < _macbethPatches.size(); p++)
    {
      outputFile << final_values[3 * p + 0] << ", " << final_values[3 * p + 1] << ", " << final_values[3 * p + 2]
                 << std::endl;
    }

    emit processProgress(100);
    emit loadingMessage("");
  });

  _imageLoadingWatcher->setFuture(imageLoading);
}


void ImageModel::saveMatrix(const QString &filename)
{
  save_xyz(filename.toStdString().c_str(), &_correctionMatrix[0], 3);
}


void ImageModel::recalculateCorrection(double exposure)
{
  _exposure = exposure;
  emit exposureChanged(_exposure);

  if (_imageEditingWatcher->isRunning())
  {
    emit _imageEditingWatcher->cancel();
    _imageEditingWatcher->waitForFinished();
  }

  if (_imageLoadingWatcher->isRunning())
  {
    _imageLoadingWatcher->waitForFinished();
  }

  if (!isImageLoaded()) return;

  if (_imageDemosaicingWatcher->isRunning())
  {
    _imageDemosaicingWatcher->waitForFinished();
  }

  QFuture<void> imageEditting = QtConcurrent::run([=]() {
    emit processProgress(0);
    emit loadingMessage(tr("Exposure correction..."));

    const float ev = std::pow(2., exposure);

    if (_isMatrixActive)
    {
      for (int y = 0; y < _image.height(); y++)
      {
        if (_imageLoadingWatcher->isCanceled()) return;
        uchar *scanline = _image.scanLine(y);

        #pragma omp parallel for
        for (int x = 0; x < _image.width(); x++)
        {
          const int px_idx = 3 * (y * _image.width() + x);


          float tmp_color[3];
          matmul(&_correctionMatrix[0], &_pixelBuffer[px_idx], tmp_color);
          XYZ_to_RGB(tmp_color, &_pixelCorrected[px_idx]);

          for (int i = 0; i < 3; i++)
          {
            scanline[3 * x + i] = 255 * to_sRGB(_pixelCorrected[px_idx + i] * ev);
          }
        }
        emit processProgress(int(100.f * float(y) / float(_image.height() - 1)));
      }
    }
    else
    {
      for (int y = 0; y < _image.height(); y++)
      {
        if (_imageLoadingWatcher->isCanceled()) return;
        uchar *scanline = _image.scanLine(y);

        #pragma omp parallel for
        for (int x = 0; x < _image.width(); x++)
        {
          const int px_idx = 3 * (y * _image.width() + x);

          for (int i = 0; i < 3; i++)
          {
            scanline[3 * x + i] = 255 * to_sRGB(_pixelBuffer[px_idx + i] * ev);
          }
        }
        emit processProgress(int(100.f * float(y) / float(_image.height() - 1)));
      }
    }


    emit imageChanged();
    emit loadingMessage("");
  });

  _imageEditingWatcher->setFuture(imageEditting);
}


float lerp(float a, float b, float t)
{
  return a + t * (b - a);
}


void ImageModel::recalculateMacbethPatches()
{
  _macbethPatches.clear();
  _macbethPatchesCenters.clear();

  std::array<cv::Point2f, 4> src {cv::Point2f(0., 0.), cv::Point2f(1., 0.), cv::Point2f(1., 1.), cv::Point2f(0., 1.)};
  std::array<cv::Point2f, 4> dest;

  for (int i = 0; i < 4; i++)
  {
    dest[i] = cv::Point2f(_macbethOutline[i].x(), _macbethOutline[i].y());
  }

  cv::Mat transform = cv::getPerspectiveTransform(&src[0], &dest[0]);

  const int n_cols  = 6;
  const int n_lines = 4;

  const float dead_width  = 1.f * _innerMarginX;
  const float dead_height = 1.f * _innerMarginY;

  const float margin_width  = dead_width / float(n_cols + 1);
  const float margin_height = dead_height / float(n_lines + 1);

  const float effective_width  = 1.f - dead_width;
  const float effective_height = 1.f - dead_height;

  const float patch_width  = effective_width / float(n_cols);
  const float patch_height = effective_height / float(n_lines);

  for (int y = 0; y < n_lines; y++)
  {
    for (int x = 0; x < n_cols; x++)
    {
      // Compute patch position in local coordinates (0..1, 0..1)
      const float x_left  = float(x + 1) * margin_width + x * patch_width;
      const float x_right = x_left + patch_width;

      const float y_top    = float(y + 1) * margin_height + y * patch_height;
      const float y_bottom = y_top + patch_height;

      const float x_center = x_left + patch_width / 2.f;
      const float y_center = y_top + patch_height / 2.f;

      // Transform perspective given the Macbeth outline
      const std::vector<cv::Point2f> patch_org = {
          cv::Point2f(x_left, y_top),
          cv::Point2f(x_right, y_top),
          cv::Point2f(x_right, y_bottom),
          cv::Point2f(x_left, y_bottom),
      };

      std::vector<cv::Point2f> patch_dest;
      cv::perspectiveTransform(patch_org, patch_dest, transform);

      QPolygonF patch;

      for (const cv::Point2f &p : patch_dest)
      {
        patch << QPointF(p.x, p.y);
      }

      _macbethPatches << patch;

      // Transformation for patch center
      const std::vector<cv::Point2f> patch_center_org = {cv::Point2f(x_center, y_center)};
      std::vector<cv::Point2f>       patch_center_dest;

      cv::perspectiveTransform(patch_center_org, patch_center_dest, transform);

      _macbethPatchesCenters << QPointF(patch_center_dest[0].x, patch_center_dest[0].y);
    }
  }

  emit macbethChartChanged();
}
