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
    REDUCE2X2,
    BARYCENTRIC2X2,
    VNG4,
    AHD,
    RCD,
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

  // NONE

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

  // BASIC

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

  // REDUCE2X2

  void reduce2x2_demosaic_rgb(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

  void reduce2x2_demosaic(
      const float *bayered_image,
      float *      debayered_image,
      size_t       width,
      size_t       height,
      unsigned int filters);

  // BARYCENTRIC2X2

  void barycentric2x2_demosaic_rgb(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

  void barycentric2x2_demosaic(
      const float *bayered_image,
      float *      debayered_image,
      size_t       width,
      size_t       height,
      unsigned int filters);

  // VGN 4

  void vng4_demosaic_rgb(
      const float *rawData,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

  void
  vgn4_demosaic(const float *bayered_image, float *debayered_image, size_t width, size_t height, unsigned int filters);

  // AHD

  void ahd_demosaic_rgb(
      const float *rawData,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       w,
      size_t       h,
      unsigned int filters);


  void ahd_demosaic(const float *bayered_image, float *debayered_image, size_t w, size_t h, unsigned int filters);

  // AMAZE

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

  // RCD

  void rcd_demosaic_rgb(
      const float *rawData,
      float *      red,
      float *      green,
      float *      blue,
      size_t       w,
      size_t       h,
      unsigned int filters);

  void rcd_demosaic(const float *bayered_image, float *debayered_image, size_t w, size_t h, unsigned int filters);

#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // DEMOSAICING_H_