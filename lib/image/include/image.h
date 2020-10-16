#ifndef IMAGE_H_
#define IMAGE_H_

#include <stddef.h>

#include <imagergb.h>
#include <imageraw.h>
#include <imageprocessing.h>

#ifdef HAS_TIFF
#  include <tiff.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
  int read_image(const char *filename, float **pixels, size_t *width, size_t *height);
  int read_image_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height);

  int write_image(const char *filename, const float *pixels, size_t width, size_t height);
  int write_image_rgb(
      const char * filename,
      const float *pixels_red,
      const float *pixels_green,
      const float *pixels_blue,
      size_t       width,
      size_t       height);
#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // IMAGE_H_
