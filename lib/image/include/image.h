#ifndef IMAGE_H_
#define IMAGE_H_

#include <stddef.h>

#ifdef __cpluplus
extern "C"
{
#endif

#ifdef HAS_TIFF
  int read_tiff(const char *filename, float **pixels, size_t *width, size_t *height);
  int write_tiff(const char *filename, const float *pixels, uint32 width, uint32 height, uint16 bps);
#endif

  // int read_png(const char *filename, float **pixels, size_t *width, size_t *height);
  // int write_png(const char *filename, const float *pixels, size_t width, size_t height);

  int read_exr(const char *filename, float **pixels, size_t *width, size_t *height);
  int write_exr(const char *filename, const float *pixels, size_t width, size_t height);

  int read_image(const char *filename, float **pixels, size_t *width, size_t *height);
  int write_image(const char *filename, const float *pixels, size_t width, size_t height);

#ifdef __cpluplus
}
#endif


#endif   // IMAGE_H_
