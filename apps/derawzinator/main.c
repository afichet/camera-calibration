#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <image.h>
#include <demosaicamaze.h>

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

  size_t           width, height;
  RAWDebayerMethod method = AMAZE;

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
    float *debayered = NULL;

    memcpy(bayered_pixels, image_r, width * height * sizeof(float));

    switch (method)
    {
      case BASIC:
        basic_debayer(bayered_pixels, image_r, image_g, image_b, width, height, 0x49494949);
        break;

      case AMAZE:
        debayered = (float *)calloc(4 * width * height, sizeof(float));

        amaze_demosaic_RT(bayered_pixels, debayered, width, height, 0x49494949);

        for (size_t row = 0; row < height; row++)
        {
          for (size_t col = 0; col < width; col++)
          {
            image_r[row * width + col] = debayered[(row * width + col) * 4 + 0];
            image_g[row * width + col] = debayered[(row * width + col) * 4 + 1];
            image_b[row * width + col] = debayered[(row * width + col) * 4 + 2];
          }
        }

        free(debayered);
        break;
    }

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
