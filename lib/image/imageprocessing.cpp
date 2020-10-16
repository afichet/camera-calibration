#include <imageprocessing.h>
#include <color-converter.h>

#include <cstring>

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

extern "C"
{
  /**
   * Demosaicing
   * ===========
   *
   * We place the Bayer samples in the target images
   * For instance, if the Bayer pattern is:
   *
   * ┌───┬───┐
   * │ G │ B │
   * ├───┼───┤
   * │ R │ G │
   * └───┴───┘
   *
   *        with the corresponding bayered image...
   *
   *                         ┌────────────────┬────────────────┐
   *                         │ (x, y)         │ (x + 1, y)     │
   *                         ├────────────────┼────────────────┤
   *                         │ (x, y + 1)     │ (x + 1, y + 1) │
   *                         └────────────────┴────────────────┘
   *
   * Each buffer is populated as follows:
   *
   *       ┌────────────────┬──────────────────┐
   *       │ 0              │ 0                │
   *   R = ├────────────────┼──────────────────┤
   *       │ px(x, y + 1)   │ 0                │
   *       └────────────────┴──────────────────┘
   *
   *       ┌────────────────┬──────────────────┐
   *       │ px(x, y)       │ 0                │
   *   G = ├────────────────┼──────────────────┤
   *       │ 0              │ px(x + 1, y + 1) │
   *       └────────────────┴──────────────────┘
   *
   *       ┌────────────────┬──────────────────┐
   *       │ 0              │ px(x + 1, y)     │
   *   B = ├────────────────┼──────────────────┤
   *       │ 0              │ 0                │
   *       └────────────────┴──────────────────┘
   *
   * Notice, G receives two values.
   *
   * Then, we perform a convolution on each channel to fill in the missing
   * values.
   *
   * The final image (width, height, 3) is reconstructed by taking for each
   * pixel the R, G, B arrays (each, (width, height)).
   */
  void basic_debayer(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height)
  {
    size_t n_elems = width * height;

    float *r_buffer = new float[n_elems];
    float *g_buffer = new float[n_elems];
    float *b_buffer = new float[n_elems];

    float *bayer[4] = {g_buffer, b_buffer, r_buffer, g_buffer};

    memset(r_buffer, 0, n_elems * sizeof(float));
    memset(g_buffer, 0, n_elems * sizeof(float));
    memset(b_buffer, 0, n_elems * sizeof(float));

    // Populate each color from the bayered image
    #pragma omp parallel for
    for (int y = 0; y < int(height); y += 2)
    {
      for (size_t x = 0; x < width; x += 2)
      {
        // clang-format off
        bayer[0][(y    ) * width + x    ] = bayered_image[(y    ) * width + x    ];
        bayer[1][(y    ) * width + x + 1] = bayered_image[(y    ) * width + x + 1];
        bayer[2][(y + 1) * width + x    ] = bayered_image[(y + 1) * width + x    ];
        bayer[3][(y + 1) * width + x + 1] = bayered_image[(y + 1) * width + x + 1];
        // clang-format on
      }
    }

    // Now, do the convolutions
    // clang-format off
    float r_matrix[9] = {
      0.25f, 0.50f, 0.25f,
      0.50f, 1.00f, 0.50f,
      0.25f, 0.50f, 0.25f
    };

    float b_matrix[9] = {
      0.25f, 0.50f, 0.25f,
      0.50f, 1.00f, 0.50f,
      0.25f, 0.50f, 0.25f
    };

    float g_matrix[9] = {
      0.00f, 0.25f, 0.00f,
      0.25f, 1.00f, 0.25f,
      0.00f, 0.25f, 0.00f
    };
    // clang-format on

    image_convolve3x3(r_matrix, r_buffer, pixels_red, width, height);
    image_convolve3x3(g_matrix, g_buffer, pixels_green, width, height);
    image_convolve3x3(b_matrix, b_buffer, pixels_blue, width, height);

    delete[] r_buffer;
    delete[] g_buffer;
    delete[] b_buffer;
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