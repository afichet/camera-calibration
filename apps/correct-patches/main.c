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

  float *patches_out = (float *)calloc(3 * size, sizeof(float));

  for (size_t i = 0; i < size; i++)
  {
    // We multiply the correction matrix with each patch color
    patches_out[3 * i]
        = matrix[0] * patches_in[3 * i] + matrix[1] * patches_in[3 * i + 1] + matrix[2] * patches_in[3 * i + 2];
    patches_out[3 * i + 1]
        = matrix[3] * patches_in[3 * i] + matrix[4] * patches_in[3 * i + 1] + matrix[5] * patches_in[3 * i + 2];
    patches_out[3 * i + 2]
        = matrix[6] * patches_in[3 * i] + matrix[7] * patches_in[3 * i + 1] + matrix[8] * patches_in[3 * i + 2];
  }

  save_xyz(argv[3], patches_out, size);

  free(patches_in);
  free(matrix);
  free(patches_out);

  return 0;
}
