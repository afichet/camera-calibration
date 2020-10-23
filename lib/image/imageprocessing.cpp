#include <imageprocessing.h>
#include <color-converter.h>

#include <iostream>
#include <cstring>

extern "C"
{
  void image_convolve3x3(float *matrix, float *array_in, float *array_out, size_t width, size_t height)
  {
    #pragma omp parallel for
    for (int y = 1; y < (int)height - 1; y++)
    {
      for (int x = 1; x < (int)width - 1; x++)
      {
        // clang-format off
        array_out[y * width + x] =
          matrix[0] * array_in[(y - 1) * width + x - 1] + matrix[1] * array_in[(y - 1) * width + x] + matrix[2] * array_in[(y - 1) * width + x + 1] +
          matrix[3] * array_in[(  y  ) * width + x - 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x + 1] +
          matrix[6] * array_in[(y + 1) * width + x - 1] + matrix[7] * array_in[(y + 1) * width + x] + matrix[8] * array_in[(y + 1) * width + x + 1];
        // clang-format on
      }
    }

    // Doing the borders

    #pragma omp parallel for
    for (int x = 1; x < (int)width - 1; x++)
    {
      int y = 0;

      // clang-format off
      array_out[y * width + x] =
        matrix[0] * array_in[(y + 1) * width + x - 1] + matrix[1] * array_in[(y + 1) * width + x] + matrix[2] * array_in[(y + 1) * width + x + 1] +
        matrix[3] * array_in[(  y  ) * width + x - 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x + 1] +
        matrix[6] * array_in[(y + 1) * width + x - 1] + matrix[7] * array_in[(y + 1) * width + x] + matrix[8] * array_in[(y + 1) * width + x + 1];
      // clang-format on

      y = height - 1;

      // clang-format off
      array_out[y * width + x] =
        matrix[0] * array_in[(y - 1) * width + x - 1] + matrix[1] * array_in[(y - 1) * width + x] + matrix[2] * array_in[(y - 1) * width + x + 1] +
        matrix[3] * array_in[(  y  ) * width + x - 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x + 1] +
        matrix[6] * array_in[(y - 1) * width + x - 1] + matrix[7] * array_in[(y - 1) * width + x] + matrix[8] * array_in[(y - 1) * width + x + 1];
      // clang-format on
    }

    #pragma omp parallel for
    for (int y = 1; y < (int)height - 1; y++)
    {
      int x = 0;

      // clang-format off
      array_out[y * width + x] =
        matrix[0] * array_in[(y - 1) * width + x + 1] + matrix[1] * array_in[(y - 1) * width + x] + matrix[2] * array_in[(y - 1) * width + x + 1] +
        matrix[3] * array_in[(  y  ) * width + x + 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x + 1] +
        matrix[6] * array_in[(y + 1) * width + x + 1] + matrix[7] * array_in[(y + 1) * width + x] + matrix[8] * array_in[(y + 1) * width + x + 1];
      // clang-format on

      x = width - 1;

      // clang-format off
      array_out[y * width + x] =
        matrix[0] * array_in[(y - 1) * width + x - 1] + matrix[1] * array_in[(y - 1) * width + x] + matrix[2] * array_in[(y - 1) * width + x - 1] +
        matrix[3] * array_in[(  y  ) * width + x - 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x - 1] +
        matrix[6] * array_in[(y + 1) * width + x - 1] + matrix[7] * array_in[(y + 1) * width + x] + matrix[8] * array_in[(y + 1) * width + x - 1];
      // clang-format on
    }

    // And the edges

    int x, y;

    x = 0;
    y = 0;

    // clang-format off
    array_out[y * width + x] =
      matrix[0] * array_in[(y + 1) * width + x + 1] + matrix[1] * array_in[(y + 1) * width + x] + matrix[2] * array_in[(y + 1) * width + x + 1] +
      matrix[3] * array_in[(  y  ) * width + x + 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x + 1] +
      matrix[6] * array_in[(y + 1) * width + x + 1] + matrix[7] * array_in[(y + 1) * width + x] + matrix[8] * array_in[(y + 1) * width + x + 1];
    // clang-format on

    // x = 0
    y = height - 1;

    // clang-format off
    array_out[y * width + x] =
      matrix[0] * array_in[(y - 1) * width + x + 1] + matrix[1] * array_in[(y - 1) * width + x] + matrix[2] * array_in[(y - 1) * width + x + 1] +
      matrix[3] * array_in[(  y  ) * width + x + 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x + 1] +
      matrix[6] * array_in[(y - 1) * width + x + 1] + matrix[7] * array_in[(y - 1) * width + x] + matrix[8] * array_in[(y - 1) * width + x + 1];
    // clang-format on

    x = width - 1;
    // y = height - 1;

    // clang-format off
    array_out[y * width + x] =
      matrix[0] * array_in[(y - 1) * width + x - 1] + matrix[1] * array_in[(y - 1) * width + x] + matrix[2] * array_in[(y - 1) * width + x - 1] +
      matrix[3] * array_in[(  y  ) * width + x - 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x - 1] +
      matrix[6] * array_in[(y - 1) * width + x - 1] + matrix[7] * array_in[(y - 1) * width + x] + matrix[8] * array_in[(y - 1) * width + x - 1];
    // clang-format on

    // x = width - 1;
    y = 0;

    // clang-format off
    array_out[y * width + x] =
      matrix[0] * array_in[(y + 1) * width + x - 1] + matrix[1] * array_in[(y + 1) * width + x] + matrix[2] * array_in[(y + 1) * width + x - 1] +
      matrix[3] * array_in[(  y  ) * width + x - 1] + matrix[4] * array_in[(  y  ) * width + x] + matrix[5] * array_in[(  y  ) * width + x - 1] +
      matrix[6] * array_in[(y + 1) * width + x - 1] + matrix[7] * array_in[(y + 1) * width + x] + matrix[8] * array_in[(y + 1) * width + x - 1];
    // clang-format on
  }

  void correct_image(float *pixels, size_t width, size_t height, float *matrix)
  {
    for (size_t i = 0; i < width * height; i++)
    {
      float tmp_color[3];
      matmul(matrix, &pixels[3 * i], tmp_color);
      XYZ_to_RGB(tmp_color, &pixels[3 * i]);
    }
  }
}