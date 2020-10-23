#ifndef DEMOSAICAMAZE_H_
#define DEMOSAICAMAZE_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus


  void amaze_demosaic_RT(const float *const in, float *out, int width, int height, const unsigned int filters);

#ifdef __cplusplus
}
#endif   // __cplusplus

#endif   // DEMOSAICAMAZE_H_