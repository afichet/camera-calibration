#ifndef IO_H_
#define IO_H_

#ifdef __cplusplus
extern "C"
{
#endif   // __cplusplus

#include <stddef.h>

  int read_spd(const char *filename, int **wavelengths, float **values, size_t *size);

  int read_cmfs(
      const char *filename,
      int **      wavelengths,
      float **    values_x,
      float **    values_y,
      float **    values_z,
      size_t *    size);

  int load_xyz(const char *filename, float **xyz, size_t *size);
  int save_xyz(const char *filename, const float *xyz, size_t size);

#ifdef __cplusplus
}
#endif   // __cplusplus


#endif   // IO_H_
