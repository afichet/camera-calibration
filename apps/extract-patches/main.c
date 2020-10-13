#include <stdio.h>
#include <stdlib.h>

#include <color-converter.h>
#include <io.h>
#include <image.h>

int load_boxfile(const char *filename, int **areas, size_t *size)
{
  FILE *fin = fopen(filename, "r");

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open file %s\n", filename);
    return -1;
  }

  int *buff_values = NULL;
  *size            = 0;

  while (!feof(fin))
  {
    (*size)++;
    int *buff_values_temp = (int *)realloc(buff_values, 4 * (*size) * sizeof(int));

    if (buff_values_temp == NULL)
    {
      fprintf(stderr, "Memory allocation error\n");
      fclose(fin);
      free(buff_values);
      return -1;
    }

    buff_values = buff_values_temp;

    int r = fscanf(
        fin,
        "%d,%d,%d,%d\n",
        &buff_values[4 * (*size - 1)],
        &buff_values[4 * (*size - 1) + 1],
        &buff_values[4 * (*size - 1) + 2],
        &buff_values[4 * (*size - 1) + 3]);

    if (r == 0)
    {
      fprintf(stderr, "Error while reading file %s\n", filename);
      fclose(fin);
      free(buff_values);
      return -1;
    }
  }

  fclose(fin);

  *areas = buff_values;

  return 0;
}


int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf(
        "Usage:\n"
        "------\n"
        "extract-patches <tiff_file> <areas> <output_csv>\n");

    return 0;
  }

  const char *filename_image = argv[1];
  const char *filename_areas = argv[2];
  const char *filename_out   = argv[3];

  float *image = NULL;
  size_t width, height;

  int *  areas = NULL;
  size_t n_areas;

  int err = load_boxfile(filename_areas, &areas, &n_areas);
  if (err != 0)
  {
    fprintf(stderr, "Cannot open area file %s\n", filename_areas);
    goto clean;
  }

  err = read_image(filename_image, &image, &width, &height);
  if (err != 0)
  {
    fprintf(stderr, "Cannot read input image file\n");
    goto clean;
  }

  float *patches = (float *)calloc(3 * n_areas, sizeof(float));

  // Average value computation for each patch
  for (size_t a = 0; a < n_areas; a++)
  {
    float patch_value[3] = {0};

    for (int y = areas[4 * a + 1]; y < areas[4 * a + 3]; y++)
    {
      for (int x = areas[4 * a]; x < areas[4 * a + 2]; x++)
      {
        for (int c = 0; c < 3; c++)
        {
          patch_value[c] += image[3 * (y * width + x) + c];
        }
      }
    }

    const float div = (areas[4 * a + 2] - areas[4 * a]) * (areas[4 * a + 3] - areas[4 * a + 1]);

    for (int i = 0; i < 3; i++)
    {
      patches[3 * a + i] = patch_value[i] / div;
    }
  }

  err = save_xyz(filename_out, patches, n_areas);

  if (err != 0)
  {
    fprintf(stderr, "Cannot save patches file\n");
    goto clean;
  }

clean:
  free(image);
  free(areas);
  free(patches);

  return err;
}
