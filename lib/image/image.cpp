#include <image.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

extern "C"
{
  int read_image(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    const size_t len = strlen(filename);

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return read_exr(filename, pixels, width, height);
    }

    if (strcmp(filename + len - 3, "txt") == 0 || strcmp(filename + len - 3, "TXT") == 0)
    {
      return read_raw(filename, pixels, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "tiff") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "tif") == 0)
    {
      return read_tiff(filename, pixels, width, height);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }


  int read_image_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height)
  {
    const size_t len = strlen(filename);

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return read_exr_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }

    if (strcmp(filename + len - 3, "txt") == 0 || strcmp(filename + len - 3, "TXT") == 0)
    {
      return read_raw_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "tiff") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "tif") == 0)
    {
      return read_tiff_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }


  int write_image(const char *filename, const float *pixels, size_t width, size_t height)
  {
    const size_t len = strlen(filename);

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return write_exr(filename, pixels, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "TIFF") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "TIF") == 0)
    {
      return write_tiff(filename, pixels, width, height, 8);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }


  int write_image_rgb(
      const char * filename,
      const float *pixels_red,
      const float *pixels_green,
      const float *pixels_blue,
      size_t       width,
      size_t       height)
  {
    const size_t len = strlen(filename);

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return write_exr_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "TIFF") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "TIF") == 0)
    {
      return write_tiff_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height, 8);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }
}
