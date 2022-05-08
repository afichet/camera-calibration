#ifndef IMAGERGB_H_
#define IMAGERGB_H_

#include <stddef.h>
#include <stdint.h>

#ifdef HAS_TIFF
#    include <tiff.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef HAS_TIFF
    /**
     * Reads a TIFF image file. Pixels are renormalized to be within 0..1.
     * For 8 bits images, a inverse sRGB gamma function is applied to get linear RGB values
     *
     * @param filename filename to read the image from
     * @param pixels pixels that are going to be allocated by the function (3 * width * height)
     * @param width gives the with of the read image
     * @param height gives the height of the read image
     *
     * @returns 0 if sucessfull
     */
    int read_tiff(const char* filename, float** pixels, size_t* width, size_t* height);

    /**
     * Reads a TIFF image file. Pixels are renormalized to be within 0..1.
     * For 8 bits images, a inverse sRGB gamma function is applied to get linear RGB values
     *
     * @param filename filename to read the image from
     * @param pixels_red red pixels that are going to be allocated by the function (width * height)
     * @param pixels_green green pixels that are going to be allocated by the function (width * height)
     * @param pixels_blue blue pixels that are going to be allocated by the function (width * height)
     * @param width gives the with of the read image
     * @param height gives the height of the read image
     *
     * @returns 0 if sucessfull
     */
    int read_tiff_rgb(
      const char* filename,
      float**     pixels_red,
      float**     piexls_green,
      float**     pixels_blus,
      size_t*     width,
      size_t*     height);

    /**
     * Writes a TIFF image file. Pixels are expected to be within 0..1.
     * For 8 bits images (bps=1), a sRGB gamma function is applied to write non linear sRGB values
     *
     * @param filename filename to write the image to
     * @param pixels pixels to write (must be at least size of 3*width*height)
     * @param width width of the image to write
     * @param height height of the image to write
     *
     * @returns 0 if sucessfull
     */
    int write_tiff(const char* filename, const float* pixels, uint32_t width, uint32_t height, uint16_t bps);

    /**
     * Writes a TIFF image file. Pixels are expected to be within 0..1.
     * For 8 bits images (bps=1), a sRGB gamma function is applied to write non linear sRGB values
     *
     * @param filename filename to write the image to
     * @param pixels_red pixels to write (must be at least size of width*height)
     * @param pixels_green pixels to write (must be at least size of width*height)
     * @param pixels_blue pixels to write (must be at least size of width*height)
     * @param width width of the image to write
     * @param height height of the image to write
     *
     * @returns 0 if successfull
     */
    int write_tiff_rgb(
      const char*  filename,
      const float* pixels_red,
      const float* pixels_green,
      const float* pixels_blue,
      uint32_t     width,
      uint32_t     height,
      uint16_t     bps);
#endif   // HAS_TIFF

    /**
     * Reads an EXR image file.
     *
     * @param filename filename to read the image from
     * @param pixels pixels that are going to be allocated by the function (3 * width * height)
     * @param width gives the with of the read image
     * @param height gives the height of the read image
     *
     * @returns 0 if sucessfull
     */
    int read_exr(const char* filename, float** pixels, size_t* width, size_t* height);

    /**
     * Reads an EXR image file.
     *
     * @param filename filename to read the image from
     * @param pixels_red red pixels that are going to be allocated by the function (width * height)
     * @param pixels_green green pixels that are going to be allocated by the function (width * height)
     * @param pixels_blue blue pixels that are going to be allocated by the function (width * height)
     * @param width gives the with of the read image
     * @param height gives the height of the read image
     *
     * @returns 0 if sucessfull
     */
    int read_exr_rgb(
      const char* filename,
      float**     pixels_red,
      float**     piexls_green,
      float**     pixels_blus,
      size_t*     width,
      size_t*     height);


    /**
     * Writes an EXR image file.
     *
     * @param filename filename to write the image to
     * @param pixels pixels to write (must be at least size of 3*width*height)
     * @param width width of the image to write
     * @param height height of the image to write
     *
     * @returns 0 if sucessfull
     */
    int write_exr(const char* filename, const float* pixels, size_t width, size_t height);

    /**
     * Writes an EXR image file.
     *
     * @param filename filename to write the image to
     * @param pixels_red pixels to write (must be at least size of width*height)
     * @param pixels_green pixels to write (must be at least size of width*height)
     * @param pixels_blue pixels to write (must be at least size of width*height)
     * @param width width of the image to write
     * @param height height of the image to write
     *
     * @returns 0 if successfull
     */
    int write_exr_rgb(
      const char*  filename,
      const float* pixels_red,
      const float* pixels_green,
      const float* pixels_blue,
      size_t       width,
      size_t       height);
#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // IMAGERGB_H_
