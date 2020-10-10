#include <stdio.h>
#include <stdlib.h>

#include <io.h>
#include <color-converter.h>
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
        "overlay-areas <image_in> <areas> <image_out>\n");

    return 0;
  }

  const char *filename_image_in  = argv[1];
  const char *filename_areas     = argv[2];
  const char *filename_image_out = argv[3];

  float *image = NULL;
  size_t width, height;
  int *  areas = NULL;
  size_t n_areas;

  // Open image in
  int err = read_image(filename_image_in, &image, &width, &height);

  if (err != 0)
  {
    fprintf(stderr, "Cannot read input image file\n");
    return -1;
  }

  // Load areas

  err = load_boxfile(filename_areas, &areas, &n_areas);
  if (err != 0)
  {
    fprintf(stderr, "Cannot open area file %s\n", filename_areas);
    free(image);
    return -1;
  }

  // Overlay
  for (size_t a = 0; a < n_areas; a++)
  {
    for (int y = areas[4 * a + 1]; y < areas[4 * a + 3]; y++)
    {
      for (int x = areas[4 * a]; x < areas[4 * a + 2]; x++)
      {
        for (int c = 1; c < 3; c++)
        {
          image[3 * (y * width + x) + c] *= .5f;
        }
      }
    }
  }

  // Image writing
  err = write_image(filename_image_out, image, width, height);

  free(image);
  free(areas);

  if (err != 0)
  {
    fprintf(stderr, "Cannot write output image file\n");
    return -1;
  }

  return 0;
}
