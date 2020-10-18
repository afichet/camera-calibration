#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <levmar.h>
#include <io.h>
#include <color-converter.h>

typedef struct
{
  size_t n_patches;
  size_t n_exposures;

  // 3 * n_patches array
  // -------------------
  // reference_patches(idx_patch, idx_color)
  //     = reference_patches[3 * idx_patch + idx_color]
  const float *reference_patches;

  // 3 * n_patches * n_exposures array
  // ---------------------------------
  // measured_patches(idx_exposure, idx_patch, idx_color)
  //     = measured_patches[3 * (idx_patch * n_exposures + idx_exposure) + idx_color]
  const float *measured_patches;

  // n_patches * n_exposures array
  // -----------------------------
  // select_patch(idx_exposure, idx_patch)
  //     = select_patch[idx_patch * n_exposures + idx_exposure]
  const int *select_patch;

  //const float *weigth_patch; // FUTURE
} fit_params;


void measure(float *pParameters, float *pMeasurements, int n_parameters, int n_measurements, void *pUserParam)
{
  (void)n_parameters;
  (void)n_measurements;

  const fit_params *info = (fit_params *)pUserParam;

  float tristim_mea_corrected[3];
  float lab_ref[3];
  float lab_mea[3];

  // First 9 parameters are the matrix components
  const float *optimized_matrix = &pParameters[0];

  // Next ones are exposition correction
  size_t       measurement_idx    = 0;

  // For each patch, we try to minimize the Delta_E_2000 between reference and measurement
  for (size_t patch_idx = 0; patch_idx < info->n_patches; patch_idx++)
  {
    const float *tristim_ref = &(info->reference_patches[3 * patch_idx]);
    const size_t offset      = patch_idx * info->n_exposures;

    for (size_t exposure_idx = 0; exposure_idx < info->n_exposures; exposure_idx++)
    {
      if (info->select_patch[offset + exposure_idx] != 0)
      {
        const float *tristim_mea = &(info->measured_patches[3 * (offset + exposure_idx)]);

        // Apply the correction matrix which is optimized by levmar
        //matmul(optimized_matrix, tristim_mea_exposed, tristim_mea_corrected);
        tristim_mea_corrected[0] = optimized_matrix[3 * exposure_idx + 6] * tristim_mea[0] + optimized_matrix[0] * tristim_mea[1] + optimized_matrix[1] * tristim_mea[2];
        tristim_mea_corrected[1] = optimized_matrix[2] * tristim_mea[0] + optimized_matrix[3 * exposure_idx + 7] * tristim_mea[1] + optimized_matrix[3] * tristim_mea[2];
        tristim_mea_corrected[2] = optimized_matrix[4] * tristim_mea[0] + optimized_matrix[5] * tristim_mea[1] + optimized_matrix[3 * exposure_idx + 8] * tristim_mea[2];

        // Transform colorspaces to Lab*
        XYZ_to_Lab(tristim_ref, lab_ref);
        XYZ_to_Lab(tristim_mea_corrected, lab_mea);

        // The error is the Delta_E_2000 between reference and corrected Lab* values
        pMeasurements[measurement_idx++] = deltaE_2000(lab_ref, lab_mea);
      }
    }
  }
}


int load_list_file(const char *filename, char ***listfile, size_t *n_elem)
{
  FILE * fin      = fopen(filename, "r");
  char **filelist = NULL;

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open file %s", filename);
    return -1;
  }

  while (!feof(fin))
  {
    (*n_elem)++;
    char **filelist_temp = (char **)realloc(filelist, (*n_elem) * sizeof(char *));

    if (filelist_temp == NULL)
    {
      fprintf(stderr, "Memory allocation error\n");
      for (size_t i = 0; i < *n_elem; i++)
      {
        free(filelist[i]);
      }

      free(filelist);
      fclose(fin);
      return -1;
    }

    filelist = filelist_temp;

    filelist[(*n_elem) - 1] = (char *)calloc(1024, sizeof(char));
    char *currEelement      = filelist[(*n_elem) - 1];

    if (currEelement == NULL)
    {
      fprintf(stderr, "Memory allocation error\n");
      for (size_t i = 0; i < *n_elem; i++)
      {
        free(filelist[i]);
      }

      free(filelist);
      fclose(fin);

      return -1;
    }

    fscanf(fin, "%s\n", currEelement);
  }

  fclose(fin);

  *listfile = filelist;

  return 0;
}


int load_patches_files(const char *filename, size_t *n_files, size_t n_patches, float **values, int **selected_patches)
{
  // TODO hard coded for now
  const float min_accepted = 0.01f;
  const float max_accepted = 0.95f;

  // Get the list of files

  *n_files        = 0;
  char **filelist = NULL;

  int ret = load_list_file(filename, &filelist, n_files);

  if (ret != 0)
  {
    fprintf(stderr, "Could not read file list %s\n", filename);
    for (size_t i = 0; i < *n_files; i++)
    {
      free(filelist[i]);
    }

    free(filelist);
    return -1;
  }

  // Read patches value for each file

  *values           = (float *)calloc(3 * (*n_files) * n_patches, sizeof(float));
  *selected_patches = (int *)calloc((*n_files) * n_patches, sizeof(int));
  memset(*selected_patches, 1, (*n_files) * n_patches * sizeof(int));

  for (size_t i = 0; i < *n_files; i++)
  {
    float *current_values = NULL;
    size_t current_size   = 0;
    int    error          = 0;

    ret = load_xyz(filelist[i], &current_values, &current_size);

    if (ret != 0)
    {
      fprintf(stderr, "Could not read file %s\n", filelist[i]);
      error = 1;
    }

    if (current_size != n_patches)
    {
      fprintf(stderr, "Number of patches missmatch for file %s\n", filelist[i]);
      error = 1;
    }

    if (error)
    {
      for (size_t i = 0; i < *n_files; i++)
      {
        free(filelist[i]);
      }

      free(filelist);
      free(current_values);
      free(*values);
      free(*selected_patches);

      return -1;
    }

    // We need to reoder the values to fit our local ordering

    for (size_t p = 0; p < n_patches; p++)
    {
      for (int c = 0; c < 3; c++)
      {
        (*values)[3 * (p * (*n_files) + i) + c] = current_values[3 * p + c];
        if (current_values[3 * p + c] < min_accepted || current_values[3 * p + c] > max_accepted)
        {
          (*selected_patches)[p * (*n_files) + i] = 0;
        }
      }
    }

    free(current_values);
  }


  for (size_t i = 0; i < *n_files; i++)
  {
    free(filelist[i]);
  }

  free(filelist);

  return 0;
}



int main(int argc, char *argv[])
{
  if (argc < 5)
  {
    printf(
        "Usage:\n"
        "------\n"
        "extract-matrix <data_xyz_ref> <list_data_measured> <output_matrix> <output_exposure>\n");

    return 0;
  }

  const char *filename_patches_reference_xyz = argv[1];
  const char *filename_patches_measured_list = argv[2];
  const char *filename_output_matrix         = argv[3];
  const char *filename_output_exposure       = argv[4];

  float *macbeth_patches_reference_xyz = NULL;
  float *macbeth_patches_measured      = NULL;
  int *  selected_patches              = NULL;
  float *mul_values                    = NULL;
  size_t n_patches                     = 0;
  size_t n_exposures                   = 0;

  float *optim_params = NULL;

  int err = load_xyz(filename_patches_reference_xyz, &macbeth_patches_reference_xyz, &n_patches);
  if (err != 0)
  {
    fprintf(stderr, "Could read reference patches file\n");
    goto error;
  }

  err = load_patches_files(
      filename_patches_measured_list,
      &n_exposures,
      n_patches,
      &macbeth_patches_measured,
      &selected_patches);
  if (err != 0)
  {
    fprintf(stderr, "Could not load measured patches files\n");
    goto error;
  }

  mul_values = (float *)calloc(3 * n_exposures, sizeof(float));

  size_t n_measurements = 0;

  // Renormalize patch values
  for (size_t idx_expo = 0; idx_expo < n_exposures; idx_expo++)
  {
    float max = 0;
    for (size_t idx_patch = 0; idx_patch < n_patches; idx_patch++)
    {
      if (selected_patches[idx_patch * n_exposures + idx_expo])
      {
        for (int c = 0; c < 3; c++)
        {
          // max = fmaxf(max, macbeth_patches_measured[3 * (idx_patch * n_exposures + idx_expo) + c]);

          ++n_measurements;
        }
      }
    }

    //   for (int c = 0; c < 3; c++)
    //   {
    //     mul_values[3 * idx_expo + c] = max;
    //   }

    //   for (size_t idx_patch = 0; idx_patch < n_patches; idx_patch++)
    //   {
    //     for (int c = 0; c < 3; c++)
    //     {
    //       macbeth_patches_measured[3 * (idx_patch * n_exposures + idx_expo) + c] /= max;
    //     }
    //   }
  }

  fit_params u_params
      = {n_patches, n_exposures, macbeth_patches_reference_xyz, macbeth_patches_measured, selected_patches};

  size_t n_params = 6 + 3 * n_exposures;
  optim_params    = (float *)calloc(n_params, sizeof(float));

  // Initialize the matrix
  // optim_params[0] = 1.f;
  optim_params[0] = 0.f;
  optim_params[1] = 0.f;

  optim_params[2] = 0.f;
  // optim_params[] = 1.f;
  optim_params[3] = 0.f;

  optim_params[4] = 0.f;
  optim_params[5] = 0.f;
  // optim_params[8] = 1.f;

  // Initialize exposure compensation values
  for (size_t i = 6; i < n_params; i++)
  {
    optim_params[i] = 1.f;
  }

  slevmar_dif(measure, optim_params, NULL, n_params, n_measurements, 1000, NULL, NULL, NULL, NULL, &u_params);

  float output_matrix[9] =
   {
    optim_params[6], optim_params[0], optim_params[1],
    optim_params[2], optim_params[7], optim_params[3],
    optim_params[4], optim_params[5], optim_params[8]
   };
  // Save the matrix
  err = save_xyz(filename_output_matrix, output_matrix, 3);

  if (err != 0)
  {
    fprintf(stderr, "Cannot write correction matrix file\n");
  }

  // Save the exposure comp
  for (size_t i = 6; i < n_params; i++)
  {
    // optim_params[i] *= mul_values[(i - 6)];
  }

  err = save_xyz(filename_output_exposure, &optim_params[6], n_exposures);

  if (err != 0)
  {
    fprintf(stderr, "Cannot write exposure compensation file\n");
  }

error:
  free(macbeth_patches_reference_xyz);
  free(macbeth_patches_measured);
  free(selected_patches);
  free(mul_values);
  free(optim_params);

  return err;
}