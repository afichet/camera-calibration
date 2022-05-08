#ifndef IMAGEPROCESSING_H_
#define IMAGEPROCESSING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus

    void image_convolve3x3(float* matrix, float* array_in, float* array_out, size_t width, size_t height);

    void correct_image(float* pixels, size_t width, size_t height, float* matrix);

#ifdef __cplusplus
}
#endif   // __cplusplus

#endif   // IMAGEPROCESSING_H_