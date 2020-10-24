#ifndef DEMOSAICING_H_
#define DEMOSAICING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus

  typedef enum
  {
    BASIC,
    VNG4,
    AMAZE,
    NONE
  } RAWDemosaicMethod;

  void demosaic_rgb(
      const float *     bayered_image,
      float *           pixels_red,
      float *           pixels_green,
      float *           pixels_blue,
      size_t            width,
      size_t            height,
      unsigned int      filters,
      RAWDemosaicMethod method);

  void demosaic(
      const float *     bayered_image,
      float *           debayered_image,
      size_t            width,
      size_t            height,
      unsigned int      filters,
      RAWDemosaicMethod method);

  void no_demosaic_rgb(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

  void
  no_demosaic(const float *bayered_image, float *debayered_image, size_t width, size_t height, unsigned int filters);

  void basic_demosaic_rgb(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

  void
  basic_demosaic(const float *bayered_image, float *debayered_image, size_t width, size_t height, unsigned int filters);

  void amaze_demosaic_rgb(
      const float *      bayered_image,
      float *            pixels_red,
      float *            pixels_green,
      float *            pixels_blue,
      size_t             width,
      size_t             height,
      const unsigned int filters);

  void amaze_demosaic(
      const float *      bayered_image,
      float *            debayered_image,
      size_t             width,
      size_t             height,
      const unsigned int filters);


  void vng4_demosaic_rgb(
      const float *rawData,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

  void vgn4_demosaic(const float *rawData, float *debayered_image, size_t width, size_t height, unsigned int filters);

#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // DEMOSAICING_H_