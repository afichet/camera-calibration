#ifndef MATRIX3X3_H_
#define MATRIX3X3_H_

float determinent(const float mat[9]);

void cofactor(const float mat_in[9], float mat_out[9]);

void transpose(const float mat_in[9], float mat_out[9]);

void adjoint(const float mat_in[9], float mat_out[9]);

void inverse(const float mat_in[9], float mat_out[9]);

void mul_mat_vec(const float mat[9], const float v[3], float v_out[3]);

void mul_mat_mat(const float mat_a[9], const float mat_b[9], float mat_out[9]);

void print_mat(const float mat[9]);

#endif   // MATRIX3X3_H_