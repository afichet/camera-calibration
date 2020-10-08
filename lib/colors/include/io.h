#ifndef IO_H_
#define IO_H_

#include <stddef.h>

void read_matrix(const char *filename, float *matrix);
void read_spd(const char *filename, int **wavelengths, float **values, size_t *size);

void read_cmfs(
    const char *filename,
    int **      wavelengths,
    float **    values_x,
    float **    values_y,
    float **    values_z,
    size_t *    size);

void load_xyz(const char *filename, float **xyz, size_t *size);
void save_xyz(const char *filename, const float *xyz, size_t size);

#endif   // IO_H_
