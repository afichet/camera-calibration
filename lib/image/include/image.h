#ifndef IMAGE_H_
#define IMAGE_H_

#include <stddef.h>



#ifdef HAS_TIFF
#  ifdef __cpluplus
extern "C"
#  endif
    int
    read_tiff(const char *filename, float **pixels, size_t *width, size_t *height);

#  ifdef __cpluplus
extern "C"
#  endif
    int
    write_tiff(const char *filename, const float *pixels, uint32 width, uint32 height, uint16 bps);
#endif

// #ifdef __cpluplus
// extern "C"
// #endif
// int read_png(const char *filename, float **pixels, size_t *width, size_t *height);

// #ifdef __cpluplus
// extern "C"
// #endif
// int write_png(const char *filename, const float *pixels, size_t width, size_t height);

#ifdef __cpluplus
extern "C"
#endif
    int
    read_exr(const char *filename, float **pixels, size_t *width, size_t *height);

#ifdef __cpluplus
extern "C"
#endif
    int
    write_exr(const char *filename, const float *pixels, size_t width, size_t height);

#ifdef __cpluplus
extern "C"
#endif
    int
    read_image(const char *filename, float **pixels, size_t *width, size_t *height);

#ifdef __cpluplus
extern "C"
#endif
    int
    write_image(const char *filename, const float *pixels, size_t width, size_t height);



#endif   // IMAGE_H_