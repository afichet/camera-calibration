
#include <stdio.h>
#include <spectrum-converter.h>
#include <util.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#ifndef min
#    define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#    define max(a, b) (((a) < (b)) ? (b) : (a))
#endif


void spectrum_emissive_to_XYZ(
  const int*   wavelengths_nm,
  const float* spectrum,
  size_t       size,
  const float* cmf_x,   // 1nm spacing
  const float* cmf_y,   // 1nm spacing
  const float* cmf_z,   // 1nm spacing
  int          cmf_first_wavelength_nm,
  size_t       cmf_size,
  float*       XYZ)
{
    memset(&XYZ[0], 0, 3 * sizeof(float));

    if (size < 2) {
        return;
    }

    const int cmf_last_wavelength = cmf_first_wavelength_nm + cmf_size - 1;
    const int start_wavelength    = max(cmf_first_wavelength_nm, wavelengths_nm[0]);
    const int end_wavelength      = min(cmf_last_wavelength, wavelengths_nm[size - 1]);

    // Early exit, selection out of range
    if (end_wavelength < start_wavelength) {
        return;
    }

    assert(start_wavelength <= end_wavelength);

    for (size_t idx_value = 0; idx_value < size - 1; idx_value++) {
        int wl_a = wavelengths_nm[idx_value];
        int wl_b = wavelengths_nm[idx_value + 1];

        // We have not reached yet the starting point
        if (start_wavelength > wl_b) {
            continue;
        }

        // We have finished the integration
        if (end_wavelength < wl_a) {
            break;
        }

        if (start_wavelength > wl_a) {
            wl_a = start_wavelength;
        }

        if (end_wavelength < wl_b) {
            wl_b = end_wavelength;
        }

        const size_t idx_cmf_start = (size_t)wl_a - cmf_first_wavelength_nm;
        size_t       idx_cmf_end   = (size_t)wl_b - cmf_first_wavelength_nm;

        // On last intervall we need to include the last wavelength of the spectrum
        if (idx_value == size - 2) {
            idx_cmf_end = idx_cmf_end + 1;
        }

        for (size_t idx_cmf = idx_cmf_start; idx_cmf < idx_cmf_end; idx_cmf++) {
            const float curr_wl    = cmf_first_wavelength_nm + idx_cmf;
            const float curr_value = interp(
              curr_wl,
              (float)wavelengths_nm[idx_value],
              (float)wavelengths_nm[idx_value + 1],
              spectrum[idx_value],
              spectrum[idx_value + 1]);

            XYZ[0] += cmf_x[idx_cmf] * curr_value;
            XYZ[1] += cmf_y[idx_cmf] * curr_value;
            XYZ[2] += cmf_z[idx_cmf] * curr_value;
        }
    }
}


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
  float*       XYZ)
{
    memset(&XYZ[0], 0, 3 * sizeof(float));

    if (size < 2) {
        return;
    }

    const int cmf_last_wavelength        = cmf_first_wavelength_nm + cmf_size - 1;
    const int illuminant_last_wavelength = illuminant_first_wavelength_nm + illuminant_size - 1;
    const int start_wavelength = max(max(illuminant_first_wavelength_nm, cmf_first_wavelength_nm), wavelengths_nm[0]);
    const int end_wavelength   = min(min(illuminant_last_wavelength, cmf_last_wavelength), wavelengths_nm[size - 1]);

    // Early exit, selection out of range
    if (end_wavelength < start_wavelength) {
        return;
    }

    assert(start_wavelength <= end_wavelength);

    float normalisation_factor = 0;

    for (size_t idx_value = 0; idx_value < size - 1; idx_value++) {
        int wl_a = wavelengths_nm[idx_value];
        int wl_b = wavelengths_nm[idx_value + 1];

        // We have not reached yet the starting point
        if (start_wavelength > wl_b) {
            continue;
        }

        // We have finished the integration
        if (end_wavelength < wl_a) {
            break;
        }

        if (start_wavelength > wl_a) {
            wl_a = start_wavelength;
        }

        if (end_wavelength < wl_b) {
            wl_b = end_wavelength;
        }

        const size_t idx_curve_start = (size_t)wl_a - cmf_first_wavelength_nm;
        size_t       idx_curve_end   = (size_t)wl_b - cmf_first_wavelength_nm;

        // On last intervall we need to include the last wavelength of the spectrum
        if (idx_value == size - 2) {
            idx_curve_end = idx_curve_end + 1;
        }

        for (size_t idx_curve = idx_curve_start; idx_curve < idx_curve_end; idx_curve++) {
            const float curr_wl = cmf_first_wavelength_nm + idx_curve;

            const size_t idx_illu_a = (size_t)curr_wl - illuminant_first_wavelength_nm;
            assert(curr_wl >= illuminant_first_wavelength_nm);
            assert(idx_illu_a < illuminant_size);

            const float illu_value = illuminant_spd[idx_illu_a];

            normalisation_factor += illu_value * cmf_y[idx_curve];   // Y

            const float curr_value = illu_value
                                     * interp(
                                       curr_wl,
                                       (float)wavelengths_nm[idx_value],
                                       (float)wavelengths_nm[idx_value + 1],
                                       spectrum[idx_value],
                                       spectrum[idx_value + 1]);

            XYZ[0] += cmf_x[idx_curve] * curr_value;
            XYZ[1] += cmf_y[idx_curve] * curr_value;
            XYZ[2] += cmf_z[idx_curve] * curr_value;
        }
    }

    for (size_t c = 0; c < 3; c++) {
        XYZ[c] /= normalisation_factor;
    }
}


void spectrum_oversample(
  const int*   wavelengths_nm,
  const float* spectrum,
  size_t       in_size,
  float**      oversampled_spectrum,
  size_t*      size_allocated)
{
    if (in_size < 2) return;

    int first_wavelength = wavelengths_nm[0];
    int last_wavelength  = wavelengths_nm[in_size - 1];

    *size_allocated = (size_t)last_wavelength - first_wavelength + 1;
    float* s        = (float*)calloc(*size_allocated, sizeof(float));

    if (s == NULL) {
        fprintf(stderr, "Memory allocation error");
        *oversampled_spectrum = NULL;
        return;
    }

    size_t idx_higher = 1;

    for (size_t i = 0; i < *size_allocated; i++) {
        const int curr_wl = first_wavelength + i;
        while (curr_wl > wavelengths_nm[idx_higher]) {
            idx_higher++;
        }

        assert(idx_higher < in_size);
        assert(curr_wl >= wavelengths_nm[idx_higher - 1]);
        assert(curr_wl <= wavelengths_nm[idx_higher]);

        s[i] = interp(
          (float)curr_wl,
          (float)wavelengths_nm[idx_higher - 1],
          (float)wavelengths_nm[idx_higher],
          spectrum[idx_higher - 1],
          spectrum[idx_higher]);
    }

    *oversampled_spectrum = s;
}
