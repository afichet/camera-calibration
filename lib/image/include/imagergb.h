#ifndef IMAGERGB_H_
#define IMAGERGB_H_

#include <stddef.h>

#ifdef HAS_TIFF
#  include <tiff.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef HAS_TIFF
  int read_tiff(const char *filename, float **pixels, size_t *width, size_t *height);
  int read_tiff_rgb(
      const char *filename,
      float **    pixels_red,
      float **    piexls_green,
      float **    pixels_blus,
      size_t *    width,
      size_t *    height);

  int write_tiff(const char *filename, const float *pixels, uint32 width, uint32 height, uint16 bps);
  int write_tiff_rgb(
      const char * filename,
      const float *pixels_red,
      const float *pixels_green,
      const float *pixels_blue,
      uint32       width,
      uint32       height,
      uint16       bps);
#endif   // HAS_TIFF

  int read_exr(const char *filename, float **pixels, size_t *width, size_t *height);
  int read_exr_rgb(
      const char *filename,
      float **    pixels_red,
      float **    piexls_green,
      float **    pixels_blus,
      size_t *    width,
      size_t *    height);

  int write_exr(const char *filename, const float *pixels, size_t width, size_t height);
  int write_exr_rgb(
      const char * filename,
      const float *pixels_red,
      const float *pixels_green,
      const float *pixels_blue,
      size_t       width,
      size_t       height);
#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // IMAGERGB_H_