#ifndef SPECTRUM_CONVERTER_H_
#define SPECTRUM_CONVERTER_H_

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus

#include <stddef.h>

    void spectrum_emissive_to_XYZ(
      const int*   wavelengths_nm,
      const float* spectrum,
      size_t       size,
      const float* cmf_x,   // 1nm spacing
      const float* cmf_y,   // 1nm spacing
      const float* cmf_z,   // 1nm spacing
      int          cmf_first_wavelength_nm,
      size_t       cmf_size,
      float*       XYZ);

    void spectrum_reflective_to_XYZ(
      const int*   wavelengths_nm,
      const float* spectrum,
      size_t       size,
      const float* cmf_x,   // 1nm spacing
      const float* cmf_y,   // 1nm spacing
      const float* cmf_z,   // 1nm spacing
      int          cmf_first_wavelength_nm,
      size_t       cmf_size,
      const float* illuminant_spd,   // 1nm spacing
      int          illuminant_first_wavelength_nm,
      size_t       illuminant_size,
      float*       XYZ);


    void spectrum_oversample(
      const int*   wavelengths_nm,
      const float* spectrum,
      size_t       in_size,
      float**      oversampled_spectrum,
      size_t*      size_allocated);

#ifdef __cplusplus
}
#endif   // __cplusplus

#endif   // SPECTRUM_CONVERTER_H_
