#ifndef IMAGE_H_
#define IMAGE_H_

#include <stddef.h>

#ifdef HAS_TIFF
#  include <tiff.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus

#ifdef HAS_TIFF
  int read_tiff(const char *filename, float **pixels, size_t *width, size_t *height);
  int write_tiff(const char *filename, const float *pixels, uint32 width, uint32 height, uint16 bps);
#endif   // HAS_TIFF

  /* int read_png(const char *filename, float **pixels, size_t *width, size_t *height); */
  /* int write_png(const char *filename, const float *pixels, size_t width, size_t height); */

  int read_exr(const char *filename, float **pixels, size_t *width, size_t *height);
  int write_exr(const char *filename, const float *pixels, size_t width, size_t height);

  int read_dat(const char *filename, float **pixels, size_t *width, size_t *height);

  int read_image(const char *filename, float **pixels, size_t *width, size_t *height);
  int write_image(const char *filename, const float *pixels, size_t width, size_t height);

  void correct_image(float *pixels, size_t width, size_t height, float *matrix);

#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // IMAGE_H_
