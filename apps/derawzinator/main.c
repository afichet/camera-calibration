#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <image.h>

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf(
        "Usage:\n"
        "------\n"
        "derawzinator <image_in> <image_out>\n");
    return 0;
  }

  const char *filename_in  = argv[1];
  const char *filename_out = argv[2];

  float *image_r = NULL;
  float *image_g = NULL;
  float *image_b = NULL;

  size_t width, height;

  int ret = read_image_rgb(filename_in, &image_r, &image_g, &image_b, &width, &height);

  if (ret != 0)
  {
    fprintf(stderr, "Could not open file: %s\n", filename_in);

    free(image_r);
    free(image_g);
    free(image_b);

    return ret;
  }

  const size_t len = strlen(filename_in);

  if (strcmp(filename_in + len - 4, "tiff") == 0 || strcmp(filename_in + len - 4, "tiff") == 0
      || strcmp(filename_in + len - 3, "tif") == 0 || strcmp(filename_in + len - 3, "tif") == 0
      || strcmp(filename_in + len - 3, "exr") == 0 || strcmp(filename_in + len - 3, "EXR") == 0)
  {
    float *bayered_pixels = (float *)calloc(width * height, sizeof(float));
    memcpy(bayered_pixels, image_r, width * height * sizeof(float));
    basic_debayer(bayered_pixels, image_r, image_g, image_b, width, height, "GBRG");
    free(bayered_pixels);
  }

  ret = write_image_rgb(filename_out, image_r, image_g, image_b, width, height);

  if (ret != 0)
  {
    fprintf(stderr, "Could not write file: %s\n", filename_out);

    free(image_r);
    free(image_g);
    free(image_b);

    return ret;
  }

  free(image_r);
  free(image_g);
  free(image_b);

  return 0;
}
