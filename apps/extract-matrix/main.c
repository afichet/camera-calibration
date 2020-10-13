#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <levmar.h>
#include <io.h>
#include <color-converter.h>

typedef struct
{
  size_t       n_patches;
  const float *reference_patches;
  const float *measured_patches;
} fit_params;


void measure(float *pParameters, float *pMeasurements, int n_parameters, int n_measurements, void *pUserParam)
{
  (void)n_parameters;
  (void)n_measurements;

  const fit_params *info = (fit_params *)pUserParam;

  float tristim_mea_corrected[3];
  float lab_ref[3];
  float lab_mea[3];

  // For each patch, we try to minimize the Delta_E_2000 between reference and measurement
  for (size_t patch_idx = 0; patch_idx < info->n_patches; patch_idx++)
  {
    const float *tristim_ref = &(info->reference_patches[3 * patch_idx]);
    const float *tristim_mea = &(info->measured_patches[3 * patch_idx]);

    // Apply the correction matrix which is optimized by levmar
    matmul(pParameters, tristim_mea, tristim_mea_corrected);

    // Transform colorspaces to Lab*
    XYZ_to_Lab(tristim_ref, lab_ref);
    XYZ_to_Lab(tristim_mea_corrected, lab_mea);

    // The error is the Delta_E_2000 between reference and corrected Lab* values
    pMeasurements[patch_idx] = deltaE_2000(lab_ref, lab_mea);
  }
}


int main(int argc, const char *argv[])
{
  if (argc < 4)
  {
    printf(
        "Usage:\n"
        "------\n"
        "extract-matrix <data_xyz_ref> <data_measured> <output_matrix>\n");

    return 0;
  }

  const char *filename_patches_reference_xyz = argv[1];
  const char *filename_patches_measured      = argv[2];
  const char *filename_output_matrix         = argv[3];

  float *macbeth_patches_reference_xyz = NULL;
  float *macbeth_patches_measured      = NULL;
  size_t size                          = 0;

  int err = load_xyz(filename_patches_reference_xyz, &macbeth_patches_reference_xyz, &size);
  if (err != 0)
  {
    fprintf(stderr, "Cannot read reference patches file\n");
    return -1;
  }

  err = load_xyz(filename_patches_measured, &macbeth_patches_measured, &size);
  if (err != 0)
  {
    fprintf(stderr, "Cannot read measured patches file\n");
    free(macbeth_patches_reference_xyz);
    return -1;
  }

  // Ensure no value is above 1
  // TODO: remove saturated values
  float max = 0;
  for (size_t i = 0; i < 3 * size; i++)
  {
    max = fmaxf(max, macbeth_patches_measured[i]);
  }

  for (size_t i = 0; i < 3 * size; i++)
  {
    macbeth_patches_measured[i] /= max;
  }

  // Fit matrix to find the transformation between measured colorspace and
  // reference color space
  float      matrix[9] = {1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f};
  fit_params u_params  = {size, macbeth_patches_reference_xyz, macbeth_patches_measured};

  slevmar_dif(measure, matrix, NULL, 9, size, 1000, NULL, NULL, NULL, NULL, &u_params);

  // Save fit result
  err = save_xyz(filename_output_matrix, matrix, 3);

  free(macbeth_patches_reference_xyz);
  free(macbeth_patches_measured);

  if (err != 0)
  {
    fprintf(stderr, "Cannot write correction matrix file\n");
  }

  return 0;
}
