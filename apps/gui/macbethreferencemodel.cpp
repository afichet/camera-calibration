#include "macbethreferencemodel.h"

extern "C"
{
#include <spectrum-converter.h>
#include <color-converter.h>
#include <macbeth-data.h>
}

MacbethReferenceModel::MacbethReferenceModel(
    const float *illuminant_spd,   // 1nm spacing
    int          illuminant_first_wavelength_nm,
    size_t       illuminant_size,
    const float *cmf_x,   // 1nm spacing
    const float *cmf_y,   // 1nm spacing
    const float *cmf_z,   // 1nm spacing
    int          cmf_first_wavelength_nm,
    size_t       cmf_size,
    QObject *    parent)
  : MacbethModel(parent)
  , _illuminantSPD(illuminant_size)
  , _illuminantFirstWavelength(illuminant_first_wavelength_nm)
  , _cmfX(cmf_size)
  , _cmfY(cmf_size)
  , _cmfZ(cmf_size)
  , _cmfFirstWavelength(cmf_first_wavelength_nm)
{
  memcpy(&_illuminantSPD[0], illuminant_spd, illuminant_size * sizeof(float));
  memcpy(&_cmfX[0], cmf_x, cmf_size * sizeof(float));
  memcpy(&_cmfY[0], cmf_y, cmf_size * sizeof(float));
  memcpy(&_cmfZ[0], cmf_z, cmf_size * sizeof(float));

  updateColors();
}


void MacbethReferenceModel::setIlluminant(
    const float *illuminant_spd,
    int          illuminant_first_wavelength_nm,
    size_t       illuminant_size)
{
  _illuminantSPD.resize(illuminant_size);
  _illuminantFirstWavelength = illuminant_first_wavelength_nm;

  memcpy(&_illuminantSPD[0], illuminant_spd, illuminant_size * sizeof(float));

  updateColors();
}


void MacbethReferenceModel::setCMFs(
    const float *cmf_x,
    const float *cmf_y,
    const float *cmf_z,
    int          cmf_first_wavelength_nm,
    size_t       cmf_size)
{
  _cmfX.resize(cmf_size);
  _cmfY.resize(cmf_size);
  _cmfZ.resize(cmf_size);
  _cmfFirstWavelength = cmf_first_wavelength_nm;

  memcpy(&_cmfX[0], cmf_x, cmf_size * sizeof(float));
  memcpy(&_cmfY[0], cmf_y, cmf_size * sizeof(float));
  memcpy(&_cmfZ[0], cmf_z, cmf_size * sizeof(float));

  updateColors();
}


void MacbethReferenceModel::updateColors()
{
  const size_t n_macbeth_wavelengths = sizeof(macbeth_wavelengths) / sizeof(int);

#pragma omp parallel for
  for (int i = 0; i < 24; i++)
  {
    spectrum_reflective_to_XYZ(
        macbeth_wavelengths,
        macbeth_patches[i],
        n_macbeth_wavelengths,
        &_cmfX[0],
        &_cmfY[0],
        &_cmfZ[0],
        _cmfFirstWavelength,
        _cmfX.size(),
        &_illuminantSPD[0],
        _illuminantFirstWavelength,
        _illuminantSPD.size(),
        &_linearColors[3 * i]);


    float temp_color[3];
    XYZ_to_RGB(&_linearColors[3 * i], temp_color);

    _tonemappedColors[i]
        = QColor(255.f * to_sRGB(temp_color[0]), 255.f * to_sRGB(temp_color[1]), 255.f * to_sRGB(temp_color[2]));
  }

  emit macbethChanged(_tonemappedColors);
}
