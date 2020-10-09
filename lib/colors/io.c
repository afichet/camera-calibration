#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef _MSC_VER
#  define _CRT_SECURE_NO_WARNINGS
#endif


int read_spd(const char *filename, int **wavelengths, float **values, size_t *size)
{
  FILE *fin = fopen(filename, "r");

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open file %s", filename);
    return -1;
  }

  int *  buff_wavelengths = NULL;
  float *buff_values      = NULL;
  *size                   = 0;

  while (!feof(fin))
  {
    (*size)++;
    int *  buff_wavelengths_temp = (int *)realloc(buff_wavelengths, (*size) * sizeof(int));
    float *buff_values_temp      = (float *)realloc(buff_values, (*size) * sizeof(float));

    if (buff_wavelengths_temp != NULL) buff_wavelengths = buff_wavelengths_temp;
    if (buff_values_temp != NULL) buff_values = buff_values_temp;

    if (buff_wavelengths_temp == NULL || buff_values_temp == NULL)
    {
      fprintf(stderr, "Memory allocation error");
      free(buff_wavelengths);
      free(buff_values);
      return -1;
    }

    buff_values = buff_values_temp;

    int r = fscanf(fin, "%d,%f\n", &buff_wavelengths[*size - 1], &buff_values[*size - 1]);

    if (r == 0)
    {
      fprintf(stderr, "Error while reading file %s", filename);
      free(buff_wavelengths);
      free(buff_values);
      fclose(fin);
      return -1;
    }
  }

  fclose(fin);

  *wavelengths = buff_wavelengths;
  *values      = buff_values;

  return 0;
}


int read_cmfs(
    const char *filename,
    int **      wavelengths,
    float **    values_x,
    float **    values_y,
    float **    values_z,
    size_t *    size)
{
  FILE *fin = fopen(filename, "r");

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open file %s", filename);
    return -1;
  }

  int *  buff_wavelengths = NULL;
  float *buff_values_x    = NULL;
  float *buff_values_y    = NULL;
  float *buff_values_z    = NULL;
  *size                   = 0;

  while (!feof(fin))
  {
    (*size)++;
    int *  buff_wavelengths_temp = (int *)realloc(buff_wavelengths, (*size) * sizeof(int));
    float *buff_values_x_temp    = (float *)realloc(buff_values_x, (*size) * sizeof(float));
    float *buff_values_y_temp    = (float *)realloc(buff_values_y, (*size) * sizeof(float));
    float *buff_values_z_temp    = (float *)realloc(buff_values_z, (*size) * sizeof(float));

    if (buff_wavelengths_temp != NULL) buff_wavelengths = buff_wavelengths_temp;
    if (buff_values_x_temp != NULL) buff_values_x = buff_values_x_temp;
    if (buff_values_y_temp != NULL) buff_values_y = buff_values_y_temp;
    if (buff_values_z_temp != NULL) buff_values_z = buff_values_z_temp;

    if (buff_wavelengths_temp == NULL || buff_values_x_temp == NULL || buff_values_y_temp == NULL
        || buff_values_z_temp == NULL)
    {
      fprintf(stderr, "Memory allocation error");
      free(buff_wavelengths);
      free(buff_values_x);
      free(buff_values_y);
      free(buff_values_z);
      return -1;
    }

    int r = fscanf(
        fin,
        "%d,%f,%f,%f\n",
        &buff_wavelengths[*size - 1],
        &buff_values_x[*size - 1],
        &buff_values_y[*size - 1],
        &buff_values_z[*size - 1]);

    if (r == 0)
    {
      fprintf(stderr, "Error while reading file %s", filename);
      free(buff_wavelengths);
      free(buff_values_x);
      free(buff_values_y);
      free(buff_values_z);
      fclose(fin);
      return -1;
    }
  }

  fclose(fin);

  *wavelengths = buff_wavelengths;
  *values_x    = buff_values_x;
  *values_y    = buff_values_y;
  *values_z    = buff_values_z;

  return 0;
}


int load_xyz(const char *filename, float **xyz, size_t *size)
{
  FILE *fin = fopen(filename, "r");

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open file %s", filename);
    return -1;
  }

  float *buff_values_xyz = NULL;
  *size                  = 0;

  while (!feof(fin))
  {
    (*size)++;
    float *buff_values_xyz_temp = (float *)realloc(buff_values_xyz, 3 * (*size) * sizeof(float));

    if (buff_values_xyz_temp == NULL)
    {
      fprintf(stderr, "Memory allocation error");
      free(buff_values_xyz);
      return -1;
    }

    buff_values_xyz = buff_values_xyz_temp;

    int r = fscanf(
        fin,
        "%f,%f,%f\n",
        &buff_values_xyz[3 * (*size - 1)],
        &buff_values_xyz[3 * (*size - 1) + 1],
        &buff_values_xyz[3 * (*size - 1) + 2]);

    if (r == 0)
    {
      fprintf(stderr, "Error while reading file %s", filename);
      free(buff_values_xyz);
      fclose(fin);
      return -1;
    }
  }

  fclose(fin);

  *xyz = buff_values_xyz;

  return 0;
}


int save_xyz(const char *filename, const float *xyz, size_t size)
{
  FILE *fout = fopen(filename, "w");

  if (fout == NULL)
  {
    fprintf(stderr, "Cannot open file %s", filename);
    return -1;
  }

  for (size_t i = 0; i < size; i++)
  {
    fprintf(fout, "%f,%f,%f\n", xyz[3 * i], xyz[3 * i + 1], xyz[3 * i + 2]);
  }

  fclose(fout);

  return 0;
}
