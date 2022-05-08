#include "matrix3x3.h"

#include <stdio.h>


float determinent(const float mat[9])
{
    float d1 = mat[4] * mat[8] - mat[5] * mat[7];
    float d2 = mat[3] * mat[8] - mat[5] * mat[6];
    float d3 = mat[3] * mat[7] - mat[4] * mat[6];

    return mat[0] * d1 - mat[1] * d2 + mat[2] * d3;
}


void cofactor(const float mat_in[9], float mat_out[9])
{
    mat_out[0] = +(mat_in[4] * mat_in[8] - mat_in[5] * mat_in[7]);
    mat_out[1] = -(mat_in[3] * mat_in[8] - mat_in[5] * mat_in[6]);
    mat_out[2] = +(mat_in[3] * mat_in[7] - mat_in[4] * mat_in[6]);

    mat_out[3] = -(mat_in[1] * mat_in[8] - mat_in[2] * mat_in[7]);
    mat_out[4] = +(mat_in[0] * mat_in[8] - mat_in[2] * mat_in[6]);
    mat_out[5] = -(mat_in[0] * mat_in[7] - mat_in[1] * mat_in[6]);

    mat_out[6] = +(mat_in[1] * mat_in[5] - mat_in[2] * mat_in[4]);
    mat_out[7] = -(mat_in[0] * mat_in[5] - mat_in[2] * mat_in[3]);
    mat_out[8] = +(mat_in[0] * mat_in[4] - mat_in[1] * mat_in[3]);
}


void transpose(const float mat_in[9], float mat_out[9])
{
    mat_out[0] = mat_in[0];
    mat_out[1] = mat_in[3];
    mat_out[2] = mat_in[6];

    mat_out[3] = mat_in[1];
    mat_out[4] = mat_in[4];
    mat_out[5] = mat_in[7];

    mat_out[6] = mat_in[2];
    mat_out[7] = mat_in[5];
    mat_out[8] = mat_in[8];
}


void adjoint(const float mat_in[9], float mat_out[9])
{
    float t[9];
    cofactor(mat_in, t);
    transpose(t, mat_out);
}


void inverse(const float mat_in[9], float mat_out[9])
{
    const float invDet = 1.f / determinent(mat_in);

    adjoint(mat_in, mat_out);

    for (int i = 0; i < 9; i++) {
        mat_out[i] *= invDet;
    }
}


void mul_mat_vec(const float mat[9], const float v[3], float v_out[3])
{
    v_out[0] = mat[0] * v[0] + mat[1] * v[1] + mat[2] * v[2];
    v_out[1] = mat[3] * v[0] + mat[4] * v[1] + mat[5] * v[2];
    v_out[2] = mat[6] * v[0] + mat[7] * v[1] + mat[8] * v[2];
}


void mul_mat_mat(const float mat_a[9], const float mat_b[9], float mat_out[9])
{
    // clang-format off
    mat_out[0] = mat_a[0] * mat_b[0] + mat_a[1] * mat_b[3] + mat_a[2] * mat_b[6];
    mat_out[1] = mat_a[0] * mat_b[1] + mat_a[1] * mat_b[4] + mat_a[2] * mat_b[7];
    mat_out[2] = mat_a[0] * mat_b[2] + mat_a[1] * mat_b[5] + mat_a[2] * mat_b[8];

    mat_out[3] = mat_a[3] * mat_b[0] + mat_a[4] * mat_b[3] + mat_a[5] * mat_b[6];
    mat_out[4] = mat_a[3] * mat_b[1] + mat_a[4] * mat_b[4] + mat_a[5] * mat_b[7];
    mat_out[5] = mat_a[3] * mat_b[2] + mat_a[4] * mat_b[5] + mat_a[5] * mat_b[8];

    mat_out[6] = mat_a[6] * mat_b[0] + mat_a[7] * mat_b[3] + mat_a[8] * mat_b[6];
    mat_out[7] = mat_a[6] * mat_b[1] + mat_a[7] * mat_b[4] + mat_a[8] * mat_b[7];
    mat_out[8] = mat_a[6] * mat_b[2] + mat_a[7] * mat_b[5] + mat_a[8] * mat_b[8];
    // clang-format on
}


void print_mat(const float mat[9])
{
    // clang-format off
    printf(
      "%f\t%f\t%f\n%f\t%f\t%f\n%f\t%f\t%f\n",
      mat[0], mat[1], mat[2],
      mat[3], mat[4], mat[5],
      mat[6], mat[7], mat[8]
    );
    // clang-format on
}
