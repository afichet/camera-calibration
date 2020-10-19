#include "macbethmeasuredmodel.h"
#include <cmath>
extern "C"
{
#include <color-converter.h>
}

MacbethMeasuredModel::MacbethMeasuredModel(QObject *parent)
  : MacbethModel(parent)
  , _isMatrixActive(true)
  , _minThreshold(0.0)
  , _maxThreshold(1.0)
{}


void MacbethMeasuredModel::setPatchesValues(const std::vector<float> &values)
{
  if (values.size() != 3 * 24)
  {
    return;
  }

  memcpy(&_linearColors[0], &values[0], 3 * 24 * sizeof(float));

  float max = 0;
  for (const auto &v : _linearColors)
  {
    max = std::max(max, v);
  }

  for (auto &v : _linearColors)
  {
    v /= max;
  }

  updateSelectedPatches();
  updateColors();
}

void MacbethMeasuredModel::setMinThreshold(double value)
{
  _minThreshold = value;
  updateSelectedPatches();
  emit macbethChanged(_tonemappedColors);
}

void MacbethMeasuredModel::setMaxThreshold(double value)
{
  _maxThreshold = value;
  updateSelectedPatches();
  emit macbethChanged(_tonemappedColors);
}

void MacbethMeasuredModel::updateSelectedPatches()
{
  for (size_t i = 0; i < _selectedPatches.size(); i++)
  {
    for (int c = 0; c < 3; c++)
    {
      if (_linearColors[3 * i + c] < _minThreshold || _linearColors[3 * i + c] > _maxThreshold)
      {
        _selectedPatches[i] = false;
      }
      else
      {
        _selectedPatches[i] = true;
      }
    }
  }

  _nSelectedPatches = 0;

  for (size_t i = 0; i < _selectedPatches.size(); i++)
  {
    if (_selectedPatches[i]) ++_nSelectedPatches;
  }
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
