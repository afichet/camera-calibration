#include <stdio.h>
#include <stdlib.h>
#include <io.h>

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf(
        "Usage:\n"
        "------\n"
        "correct-patches <data_xyz> <matrix> <out_data_xyz>\n");

    return 0;
  }

  float *patches_in = NULL;
  float *matrix     = NULL;
  size_t size       = 0;
  size_t mat_size;

  load_xyz(argv[1], &patches_in, &size);
  load_xyz(argv[2], &matrix, &mat_size);

  if (patches_in == NULL || matrix == NULL)
  {
    fprintf(stderr, "Cannot open patches or matrix file\n");
    free(patches_in);
    free(matrix);
    return -1;
  }

  float *patches_out = (float *)calloc(3 * size, sizeof(float));

  for (size_t i = 0; i < size; i++)
  {
    // We multiply the correction matrix with each patch color
    matmul(matrix, &patches_in[3 * i], &patches_out[3 * i]);
  }

  save_xyz(argv[3], patches_out, size);

  free(patches_in);
  free(matrix);
  free(patches_out);

  return 0;
}
