#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <image.h>
#include <demosaicing.h>

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf(
        "Usage:\n"
        "------\n"
        "derawzinator <image_in> <image_out> [Method]\n"
        "Demosaicing method is optional. It can be:\n"
        "  - NONE\n"
        "  - BASIC (bilinear)\n"
        "  - REDUCE2X2\n"
        "  - BARYCENTRIC2X2\n"
        "  - VNG4\n"
        "  - AHD\n"
        "  - RCD\n"
        "  - AMAZE (default)\n");
    return 0;
  }

  const char *filename_in  = argv[1];
  const char *filename_out = argv[2];

  unsigned int filters;
  size_t       width, height;

  float *bayered_pixels = NULL;
  float *image_r        = NULL;
  float *image_g        = NULL;
  float *image_b        = NULL;

  int          ret = 0;
  const size_t len = strlen(filename_in);

  RAWDemosaicMethod method = AMAZE;

  if (argc > 3)
  {
    const char *demosaicing_method_arg = argv[3];

    if (strcmp(demosaicing_method_arg, "NONE") == 0)
    {
      method = NONE;
    }
    else if (strcmp(demosaicing_method_arg, "BASIC") == 0)
    {
      method = BASIC;
    }
    else if (strcmp(demosaicing_method_arg, "REDUCE2X2") == 0)
    {
      method = REDUCE2X2;
    }
    else if (strcmp(demosaicing_method_arg, "BARYCENTRIC2X2") == 0)
    {
      method = BARYCENTRIC2X2;
    }
    else if (strcmp(demosaicing_method_arg, "VNG4") == 0)
    {
      method = VNG4;
    }
    else if (strcmp(demosaicing_method_arg, "AHD") == 0)
    {
      method = AHD;
    }
    else if (strcmp(demosaicing_method_arg, "RCD") == 0)
    {
      method = RCD;
    }
    else if (strcmp(demosaicing_method_arg, "AMAZE") == 0)
    {
      method = AMAZE;
    }
    else
    {
      fprintf(stderr, "Unknown method specified, default to AMAZE\n");
    }
  }

  if (strcmp(filename_in + len - 4, "tiff") == 0 || strcmp(filename_in + len - 4, "tiff") == 0
      || strcmp(filename_in + len - 3, "tif") == 0 || strcmp(filename_in + len - 3, "tif") == 0
      || strcmp(filename_in + len - 3, "exr") == 0 || strcmp(filename_in + len - 3, "EXR") == 0)
  {
    ret     = read_image_rgb(filename_in, &bayered_pixels, &image_g, &image_b, &width, &height);
    filters = 0x49494949;

    free(image_g);
    image_g = NULL;
    free(image_b);
    image_b = NULL;
  }
  else if (strcmp(filename_in + len - 3, "txt") == 0 || strcmp(filename_in + len - 3, "TXT") == 0)
  {
    ret = read_raw_file(filename_in, &bayered_pixels, &width, &height, &filters);
  }
  else
  {
    fprintf(stderr, "This input file extension is not supported: %s\n", filename_in);
    ret = -1;
  }

  if (ret == 0)
  {
    size_t image_out_width  = 0;
    size_t image_out_height = 0;

    if (method != REDUCE2X2 && method != BARYCENTRIC2X2)
    {
      image_out_width  = width;
      image_out_height = height;
    }
    else
    {
      image_out_width  = width / 2;
      image_out_height = height / 2;
    }

    const size_t image_out_size = image_out_width * image_out_height;

    image_r = (float *)calloc(image_out_size, sizeof(float));
    image_g = (float *)calloc(image_out_size, sizeof(float));
    image_b = (float *)calloc(image_out_size, sizeof(float));

    demosaic_rgb(bayered_pixels, image_r, image_g, image_b, width, height, filters, method);

    ret = write_image_rgb(filename_out, image_r, image_g, image_b, image_out_width, image_out_height);

    if (ret != 0)
    {
      fprintf(stderr, "Could not write file: %s\n", filename_out);
    }
  }
  else
  {
    fprintf(stderr, "Could not open file: %s\n", filename_in);
  }

  free(bayered_pixels);

  free(image_r);
  free(image_g);
  free(image_b);

  return ret;
}
