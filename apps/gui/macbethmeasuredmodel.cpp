#include "macbethmeasuredmodel.h"

extern "C"
{
#include <color-converter.h>
}

MacbethMeasuredModel::MacbethMeasuredModel(QObject *parent): MacbethModel(parent), _isMatrixActive(true) {}


void MacbethMeasuredModel::setPatchesValues(const std::vector<float> &values)
{
  if (values.size() != 3 * 24)
  {
    return;
  }

  memcpy(&_linearColors[0], &values[0], 3 * 24 * sizeof(float));

  updateColors();
}


void MacbethMeasuredModel::setMatrixActive(bool active)
{
  if (_isMatrixActive == active) return;
  _isMatrixActive = active;
  emit matrixActivationStateChanged(_isMatrixActive);
  updateColors();
}


void MacbethMeasuredModel::updateColors()
{
  if (_isMatrixActive)
  {
#pragma omp parallel for
    for (int i = 0; i < 24; i++)
    {
      float tmp_color_XYZ[3], tmp_color_RGB[3];
      matmul(&_correctionMatrix[0], &_linearColors[3 * i], tmp_color_XYZ);
      XYZ_to_RGB(tmp_color_XYZ, tmp_color_RGB);

      _tonemappedColors[i] = QColor(
          255.f * to_sRGB(tmp_color_RGB[0]),
          255.f * to_sRGB(tmp_color_RGB[1]),
          255.f * to_sRGB(tmp_color_RGB[2]));
    }
  }
  else
  {
#pragma omp parallel for
    for (int i = 0; i < 24; i++)
    {
      _tonemappedColors[i] = QColor(
          255.f * to_sRGB(_linearColors[3 * i]),
          255.f * to_sRGB(_linearColors[3 * i + 1]),
          255.f * to_sRGB(_linearColors[3 * i + 2]));
    }
  }

  _ready = true;
  emit macbethChanged(_tonemappedColors);
}


void MacbethMeasuredModel::setMatrix(const std::array<float, 9> matrix)
{
  if (_correctionMatrix == matrix) return;
  _correctionMatrix = matrix;
  emit matrixChanged(_correctionMatrix);
  updateColors();
}
