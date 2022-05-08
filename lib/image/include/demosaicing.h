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
      const float*      bayered_image,
      float*            pixels_red,
      float*            pixels_green,
      float*            pixels_blue,
      size_t            width,
      size_t            height,
      unsigned int      filters,
      RAWDemosaicMethod method);

    void demosaic(
      const float*      bayered_image,
      float*            debayered_image,
      size_t            width,
      size_t            height,
      unsigned int      filters,
      RAWDemosaicMethod method);

    // NONE

    void no_demosaic_rgb(
      const float* bayered_image,
      float*       pixels_red,
      float*       pixels_green,
      float*       pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

    void
    no_demosaic(const float* bayered_image, float* debayered_image, size_t width, size_t height, unsigned int filters);

    // BASIC

    void basic_demosaic_rgb(
      const float* bayered_image,
      float*       pixels_red,
      float*       pixels_green,
      float*       pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

    void basic_demosaic(
      const float* bayered_image, float* debayered_image, size_t width, size_t height, unsigned int filters);

    // REDUCE2X2

    /**
     * @brief Reduce 2x2 algorithm for demosaicing
     *
     * As the name suggests, this demosaicing technique deacreases the image
     * resolution by a factor of 4. You must provide then pixels_red,
     * pixels_green and pixels_blue buffer with size (width/2*height/2).
     *
     * @param bayered_image The bayered image
     * @param pixels_red A buffer to write the demosaiced image (/4 resolution)
     * @param pixels_green A buffer to write the demosaiced image (/4 resolution)
     * @param pixels_blue A buffer to write the demosaiced image (/4 resolution)
     * @param width Original width of the bayered image
     * @param height Original height of the bayered image
     * @param filters Arrangement of the Bayer pattern
     */
    void reduce2x2_demosaic_rgb(
      const float* bayered_image,
      float*       pixels_red,
      float*       pixels_green,
      float*       pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

    /**
     * @brief Reduce 2x2 algorithm for demosaicing
     *
     * As the name suggests, this demosaicing technique deacreases the image
     * resolution by a factor of 4. You must provide then debayered_image
     * buffer with size 3*(width/2*height/2).
     *
     * @param bayered_image The bayered image
     * @param debayered_image A buffer to output the debayered image (/4)
     * @param width Original width of the bayered image
     * @param height Original height of the bayered image
     * @param filters Arrangement of the Bayer pattern
     */
    void reduce2x2_demosaic(
      const float* bayered_image, float* debayered_image, size_t width, size_t height, unsigned int filters);

    // BARYCENTRIC2X2

    /**
     * @brief Barycentric 2x2 algorithm for demosaicing
     *
     * As the name suggests, this demosaicing technique deacreases the image
     * resolution by a factor of 4. You must provide then pixels_red,
     * pixels_green and pixels_blue buffer with size (width/2*height/2).
     *
     * @param bayered_image The bayered image
     * @param pixels_red A buffer to write the demosaiced image (/4 resolution)
     * @param pixels_green A buffer to write the demosaiced image (/4 resolution)
     * @param pixels_blue A buffer to write the demosaiced image (/4 resolution)
     * @param width Original width of the bayered image
     * @param height Original height of the bayered image
     * @param filters Arrangement of the Bayer pattern
     */
    void barycentric2x2_demosaic_rgb(
      const float* bayered_image,
      float*       pixels_red,
      float*       pixels_green,
      float*       pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

    /**
     * @brief Barycentric 2x2 algorithm for demosaicing
     *
     * As the name suggests, this demosaicing technique deacreases the image
     * resolution by a factor of 4. You must provide then debayered_image
     * buffer with size 3*(width/2*height/2).
     *
     * @param bayered_image The bayered image
     * @param debayered_image A buffer to output the debayered image (/4)
     * @param width Original width of the bayered image
     * @param height Original height of the bayered image
     * @param filters Arrangement of the Bayer pattern
     */
    void barycentric2x2_demosaic(
      const float* bayered_image, float* debayered_image, size_t width, size_t height, unsigned int filters);

    // VGN 4

    void vng4_demosaic_rgb(
      const float* rawData,
      float*       pixels_red,
      float*       pixels_green,
      float*       pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters);

    void vgn4_demosaic(
      const float* bayered_image, float* debayered_image, size_t width, size_t height, unsigned int filters);

    // AHD

    void ahd_demosaic_rgb(
      const float* rawData,
      float*       pixels_red,
      float*       pixels_green,
      float*       pixels_blue,
      size_t       w,
      size_t       h,
      unsigned int filters);


    void ahd_demosaic(const float* bayered_image, float* debayered_image, size_t w, size_t h, unsigned int filters);

    // AMAZE

    void amaze_demosaic_rgb(
      const float*       bayered_image,
      float*             pixels_red,
      float*             pixels_green,
      float*             pixels_blue,
      size_t             width,
      size_t             height,
      const unsigned int filters);

    void amaze_demosaic(
      const float* bayered_image, float* debayered_image, size_t width, size_t height, const unsigned int filters);

    // RCD

    void rcd_demosaic_rgb(
      const float* rawData, float* red, float* green, float* blue, size_t w, size_t h, unsigned int filters);

    void rcd_demosaic(const float* bayered_image, float* debayered_image, size_t w, size_t h, unsigned int filters);

#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // DEMOSAICING_H_