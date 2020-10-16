#ifndef IMAGEPROCESSING_H_
#define IMAGEPROCESSING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus

  void basic_debayer(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height);

  void correct_image(float *pixels, size_t width, size_t height, float *matrix);

#ifdef __cplusplus
}
#endif   // __cplusplus

#endif   // IMAGEPROCESSING_H_