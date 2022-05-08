#ifndef IMAGEDNG_H_
#define IMAGEDNG_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus

    // Calculate the bayer pattern color from the row and column
    inline unsigned int FC(const size_t row, const size_t col, const uint32_t filters)
    {
        return filters >> (((row << 1 & 14) + (col & 1)) << 1) & 3;
    }

    int write_dng(
      const char*    filename,
      float*         image_data,
      size_t         width,
      size_t         height,
      const uint32_t filters,
      const float    cam_xyz[9]);


#ifdef __cplusplus
}
#endif   // __cplusplus

#endif   // IMAGEDNG_H_