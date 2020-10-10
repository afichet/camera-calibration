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

  std::array<cv::Point2f, 4> src {cv::Point2f(0., 0.), cv::Point2f(1., 0.), cv::Point2f(1., 1.), cv::Point2f(0., 1.)};

  std::array<cv::Point2f, 4> dest;

  for (int i = 0; i < 4; i++)
  {
    dest[i] = cv::Point2f(_macbethOutline[i].x(), _macbethOutline[i].y());
  }

  cv::Mat transform = cv::getPerspectiveTransform(&src[0], &dest[0]);

  const int n_cols  = 6;
  const int n_lines = 4;

  const float margin_x = _innerMarginX / (n_cols + 1);
  const float margin_y = _innerMarginY / (n_lines + 1);

  const float usable_space_x = 1.f - _innerMarginX;
  const float usable_space_y = 1.f - _innerMarginY;

  const float patch_width  = usable_space_x / n_cols;
  const float patch_height = usable_space_y / n_lines;

  const float patch_footprint_x = patch_width + margin_x;
  const float patch_footprint_y = patch_height + margin_y;

  for (int y = 0; y < n_lines; y++)
  {
    for (int x = 0; x < n_cols; x++)
    {
      const float x_left  = margin_x + x * patch_footprint_x;
      const float x_right = x_left + patch_width;

      const float y_top    = margin_y + y * patch_footprint_y;
      const float y_bottom = y_top + patch_height;

      QPolygonF patch;

      std::vector<cv::Point2f> patch_org = {
          cv::Point2f(x_left, y_top),
          cv::Point2f(x_right, y_top),
          cv::Point2f(x_right, y_bottom),
          cv::Point2f(x_left, y_bottom),
      };

      std::vector<cv::Point2f> patch_dest;
      cv::perspectiveTransform(patch_org, patch_dest, transform);

      for (const cv::Point2f &p : patch_dest)
      {
        patch << QPointF(p.x, p.y);
      }

      _macbethPatches << patch;
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
