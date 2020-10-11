#include "macbethmodel.h"

#include <cstddef>
#include <cmath>
#include <array>
#include <fstream>

extern "C"
{
#include <image.h>
#include <color-converter.h>
}

#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

MacbethModel::MacbethModel(): QObject(), _pixelBuffer(nullptr), _innerMarginX(0.01), _innerMarginY(0.01)
{
  _macbethOutline << QPointF(0, 0) << QPointF(100, 0) << QPointF(100, 100) << QPointF(0, 100);

  recalculateMacbethPatches();
}

MacbethModel::~MacbethModel()
{
  delete _pixelBuffer;
}

void MacbethModel::openFile(const QString &filename)
{
  size_t width, height;

  read_image(filename.toStdString().c_str(), &_pixelBuffer, &width, &height);

  _image = QImage(width, height, QImage::Format_RGB888);

  #pragma omp parallel for
  for (size_t y = 0; y < height; y++)
  {
    uchar *scanline = _image.scanLine(y);
    for (size_t x = 0; x < width; x++)
    {
      for (size_t i = 0; i < 3; i++)
      {
        scanline[3 * x + i] = 255 * to_sRGB(_pixelBuffer[3 * (y * width + x) + i]);
      }
    }
  }

  emit imageChanged();
  emit imageLoaded(width, height);
  _macbethOutline.clear();
  _macbethOutline << QPointF(0, 0) << QPointF(_image.width(), 0) << QPointF(_image.width(), _image.height())
                  << QPointF(0, _image.height());

  recalculateMacbethPatches();
}


float lerp(float a, float b, float t)
{
  return a + t * (b - a);
}

void MacbethModel::recalculateMacbethPatches()
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

  const float margin_width = dead_width / float(n_cols + 1);
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
        const float x_left = float(x + 1) * margin_width + x * patch_width;
        const float x_right = x_left + patch_width;

        const float y_top = float(y + 1) * margin_height + y * patch_height;
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
      std::vector<cv::Point2f> patch_center_dest;

      cv::perspectiveTransform(patch_center_org, patch_center_dest, transform);

      _macbethPatchesCenters << QPointF(patch_center_dest[0].x, patch_center_dest[0].y);
    }
  }

  emit macbethChartChanged();
}

void MacbethModel::setInnerMarginX(float position)
{
  _innerMarginX = position;
  recalculateMacbethPatches();
}

void MacbethModel::setInnerMarginY(float position)
{
  _innerMarginY = position;
  recalculateMacbethPatches();
}

void MacbethModel::setOutlinePosition(int index, QPointF position)
{
  _macbethOutline[index] = position;
  recalculateMacbethPatches();
}

void MacbethModel::setExposure(double value)
{
  if (_pixelBuffer == nullptr) return;

  #pragma omp parallel for
  for (int y = 0; y < _image.height(); y++)
  {
    uchar *scanline = _image.scanLine(y);
    for (int x = 0; x < _image.width(); x++)
    {
      for (int i = 0; i < 3; i++)
      {
        scanline[3 * x + i] = 255 * to_sRGB(_pixelBuffer[3 * (y * _image.width() + x) + i] * std::pow(2., value));
      }
    }
  }

  emit imageChanged();
}

void MacbethModel::savePatches(const QString &filename)
{
  if (_pixelBuffer == nullptr) return;

  std::vector<float> patches_values(4 * _macbethPatches.size(), 0.f);

  #pragma omp parallel for
  for (int y = 0; y < _image.height(); y++)
  {
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
  }

  std::vector<float> final_values(3 * _macbethPatches.size());

  #pragma omp parallel for
  for (int p = 0; p < _macbethPatches.size(); p++)
  {
    for (int c = 0; c < 3; c++)
    {
      final_values[3 * p + c] = patches_values[4 * p + c] / patches_values[4 * p + 3];
    }
  }

  std::ofstream outputFile(filename.toStdString());

  for (int p = 0; p < _macbethPatches.size(); p++)
  {
    outputFile << final_values[3 * p + 0] << ", " << final_values[3 * p + 1] << ", " << final_values[3 * p + 2]
               << std::endl;
  }
}
