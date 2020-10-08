#include <stdio.h>
#include <stdlib.h>

#include <spectrum-converter.h>
#include <io.h>
#include <macbeth-data.h>
#include <math.h>

void get_references_patches_xyz(
    const float *cmf_x,   // 1nm spacing
    const float *cmf_y,   // 1nm spacing
    const float *cmf_z,   // 1nm spacing
    int          cmf_first_wavelength_nm,
    size_t       cmf_size,
    const float *illuminant_spd,   // 1nm spacing
    int          illuminant_first_wavelength_nm,
    size_t       illuminant_size,
    float *      patches_xyz)
{
  const size_t n_macbeth_wavelengths = sizeof(macbeth_wavelengths) / sizeof(int);
  const size_t n_macbeth_patches     = sizeof(macbeth_patches) / (n_macbeth_wavelengths * sizeof(float));

  for (size_t i = 0; i < n_macbeth_patches; i++)
  {
    spectrum_reflective_to_XYZ(
        macbeth_wavelengths,
        macbeth_patches[i],
        n_macbeth_wavelengths,
        cmf_x,
        cmf_y,
        cmf_z,
        cmf_first_wavelength_nm,
        cmf_size,
        illuminant_spd,
        illuminant_first_wavelength_nm,
        illuminant_size,
        &patches_xyz[3 * i]);
  }
}


int main(int argc, const char *argv[])
{
  if (argc < 4)
  {
    printf(
        "Usage:\n"
        "------\n"
        "gen-ref-colorchart <illuminant_spd> <cmfs> <output_data_xyz>\n");

    return 0;
  }

  // Load illuminant
  int *  wavelengths_illu_raw = NULL;
  float *values_illu_raw      = NULL;
  size_t size_illu_raw        = 0;

  read_spd(argv[1], &wavelengths_illu_raw, &values_illu_raw, &size_illu_raw);

  float *values_illu           = NULL;
  size_t size_illu             = 0;
  int    first_wavelength_illu = wavelengths_illu_raw[0];

  spectrum_oversample(wavelengths_illu_raw, values_illu_raw, size_illu_raw, &values_illu, &size_illu);

  free(wavelengths_illu_raw);
  free(values_illu_raw);

  // Load color matching functions
  int *  wavelengths_cmfs_raw = NULL;
  float *values_cmfs_x_raw    = NULL;
  float *values_cmfs_y_raw    = NULL;
  float *values_cmfs_z_raw    = NULL;
  size_t size_cmfs_raw        = 0;

  read_cmfs(argv[2], &wavelengths_cmfs_raw, &values_cmfs_x_raw, &values_cmfs_y_raw, &values_cmfs_z_raw, &size_cmfs_raw);

  float *values_cmfs_x         = NULL;
  float *values_cmfs_y         = NULL;
  float *values_cmfs_z         = NULL;
  size_t size_cmfs             = 0;
  int    first_wavelength_cmfs = wavelengths_cmfs_raw[0];

  spectrum_oversample(wavelengths_cmfs_raw, values_cmfs_x_raw, size_cmfs_raw, &values_cmfs_x, &size_cmfs);
  spectrum_oversample(wavelengths_cmfs_raw, values_cmfs_y_raw, size_cmfs_raw, &values_cmfs_y, &size_cmfs);
  spectrum_oversample(wavelengths_cmfs_raw, values_cmfs_z_raw, size_cmfs_raw, &values_cmfs_z, &size_cmfs);

  float macbeth_patches_xyz[24 * 3] = {0};
  
  get_references_patches_xyz(
      values_cmfs_x,
      values_cmfs_y,
      values_cmfs_z,
      first_wavelength_cmfs,
      size_cmfs,
      values_illu,
      first_wavelength_illu,
      size_illu,
      macbeth_patches_xyz);

  save_xyz(argv[3], macbeth_patches_xyz, 24);

  free(wavelengths_cmfs_raw);
  free(values_cmfs_x_raw);
  free(values_cmfs_y_raw);
  free(values_cmfs_z_raw);

  free(values_illu);
  free(values_cmfs_x);
  free(values_cmfs_y);
  free(values_cmfs_z);

  return 0;
}
