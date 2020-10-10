#include "macbethmodel.h"
// #include <image.h>
#include <cstddef>
#include <cmath>

extern "C"
{
#include <image.h>
#include <color-converter.h>
}

MacbethModel::MacbethModel()
  : QObject()
  , _pixelBuffer(nullptr)
  , _outerMarginX(0.01)
  , _outerMarginY(0.01)
  , _innerMarginX(0.01)
  , _innerMarginY(0.01)
  , _macbethOutline(QRectF(0, 0, 1, 1))
{}

MacbethModel::~MacbethModel()
{
  delete _pixelBuffer;
}

void MacbethModel::openFile(const QString &filename)
{
  size_t width, height;

  read_image(filename.toStdString().c_str(), &_pixelBuffer, &width, &height);

  _image = QImage(width, height, QImage::Format_RGB888);

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

  _macbethOutline.clear();
  _macbethOutline << QPointF(0, 0);
  _macbethOutline << QPointF(_image.width(), 0);
  _macbethOutline << QPointF(_image.width(), _image.height());
  _macbethOutline << QPointF(0, _image.height());

  recalculateMacbethPatches();
}


float lerp(float a, float b, float t)
{
  return a + t * (b - a);
}

void MacbethModel::recalculateMacbethPatches()
{
  _macbethPatches.clear();

  const QPointF &topLeft     = _macbethOutline[0];
  const QPointF &topRight    = _macbethOutline[1];
  const QPointF &bottomRight = _macbethOutline[2];
  const QPointF &bottomLeft  = _macbethOutline[3];

  const int n_cols  = 6;
  const int n_lines = 4;

  const float usable_space_x = 1.f - 2. * _outerMarginX;
  const float usable_space_y = 1.f - 2. * _outerMarginY;

  const float sepX = (_innerMarginX / (n_cols - 1)) * usable_space_x;
  const float sepY = (_innerMarginY / (n_lines - 1)) * usable_space_y;

  const float patch_footprint_x = usable_space_x / n_cols;
  const float patch_footprint_y = usable_space_y / n_lines;

  const float patch_width  = patch_footprint_x - sepX;
  const float patch_height = patch_footprint_y - sepY;

  for (int y = 0; y < n_lines; y++)
  {
    for (int x = 0; x < n_cols; x++)
    {
      const float x_left  = _outerMarginX + x * patch_footprint_x + sepX / 2.f;
      const float x_right = x_left + patch_width - sepX / 2.f;

      const float y_top    = _outerMarginY + y * patch_footprint_y + sepY / 2.f;
      const float y_bottom = y_top + patch_height - sepY / 2.f;

      const float x_top_left_interp  = lerp(topLeft.x(), topRight.x(), x_left);
      const float x_top_right_interp = lerp(topLeft.x(), topRight.x(), x_right);

      const float x_bottom_left_interp  = lerp(bottomLeft.x(), bottomRight.x(), x_left);
      const float x_bottom_right_interp = lerp(bottomLeft.x(), bottomRight.x(), x_right);

      const float y_left_top_interp    = lerp(topLeft.y(), bottomLeft.y(), y_top);
      const float y_left_bottom_interp = lerp(topLeft.y(), bottomLeft.y(), y_bottom);

      const float y_right_top_interp    = lerp(topRight.y(), bottomRight.y(), y_top);
      const float y_right_bottom_interp = lerp(topRight.y(), bottomRight.y(), y_bottom);


      const float x_top_left    = lerp(x_top_left_interp, x_bottom_left_interp, y_top);
      const float x_bottom_left = lerp(x_top_left_interp, x_bottom_left_interp, y_bottom);

      const float x_top_right    = lerp(x_top_right_interp, x_bottom_right_interp, y_top);
      const float x_bottom_right = lerp(x_top_right_interp, x_bottom_right_interp, y_bottom);

      const float y_top_left  = lerp(y_left_top_interp, y_right_top_interp, x_left);
      const float y_top_right = lerp(y_left_top_interp, y_right_top_interp, x_right);

      const float y_bottom_left  = lerp(y_left_bottom_interp, y_right_bottom_interp, x_left);
      const float y_bottom_right = lerp(y_left_bottom_interp, y_right_bottom_interp, x_right);

      QPolygonF patch;
      patch << QPointF(x_top_left, y_top_left);
      patch << QPointF(x_top_right, y_top_right);
      patch << QPointF(x_bottom_right, y_bottom_right);
      patch << QPointF(x_bottom_left, y_bottom_left);

      _macbethPatches << patch;
    }
  }

  emit macbethChartChanged();
}

void MacbethModel::setOuterMarginX(float position)
{
  _outerMarginX = .5f * position;
  recalculateMacbethPatches();
}

void MacbethModel::setOuterMarginY(float position)
{
  _outerMarginY = .5f * position;
  recalculateMacbethPatches();
}

void MacbethModel::setInnerMarginX(float position)
{
  _innerMarginX = .5f * position;
  recalculateMacbethPatches();
}

void MacbethModel::setInnerMarginY(float position)
{
  _innerMarginY = .5f * position;
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
