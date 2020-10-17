#ifndef IMAGERAW_H_
#define IMAGERAW_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus
  typedef struct
  {
    int   ledIdx;
    float exposureTime;
    float aperture;
    float gain;
    int   bitDepth;
    char *bayerPattern;

    char *filename_image;
    char *filename_info;
  } RAWMetadata;

  int read_raw_metadata(const char *filename, RAWMetadata *metadata);

  int read_raw(const char *filename, float **pixels, size_t *width, size_t *height);
  int read_raw_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height);

  int read_dat(const char *filename, float **pixels, size_t *width, size_t *height, size_t bit_depth);

#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // IMAGERAW_H_
