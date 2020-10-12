#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <image.h>

#include <chrono>
#include <iostream>

void convolve3x3(float *matrix, float *array_in, float *array_out, size_t width, size_t height)
{
#pragma omp parallel for schedule(static)
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
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf(
        "Usage:\n"
        "------\n"
        "convert-raw <image_in> <image_out>\n");
    return 0;
  }

  const char *filename_in  = argv[1];
  const char *filename_out = argv[2];

  FILE *fin = fopen(filename_in, "rb");

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open image file %s", filename_in);
    return -1;
  }


  /**
   * Image format
   * ============
   *
   * File content
   * ------------
   *
   * ┌──────────────┬──────────────────────────┐
   * │ Type         │ Description              │
   * ├──────────────┼──────────────────────────┤
   * │ char         │ datatype                 │
   * │ unsigned int │ height                   │
   * │ unsigned int │ width                    │
   * │ unsigned int │ number of channers       │
   * │ void*        │ image data               │
   * └──────────────┴──────────────────────────┘
   *
   * Data types
   * ----------
   *
   * ┌───────┬────────────────┐
   * │ Value │ Type           │
   * ├───────┼────────────────┤
   * │ 1     │ BOOL           │
   * │ 2     │ unsigned char  │
   * │ 3     │ char           │
   * │ 4     │ unsigned short │
   * │ 5     │ short          │
   * │ 6     │ unsigned int   │
   * │ 7     │ int            │
   * │ 8     │ float          │
   * │ 9     │ double         │
   * └───────┴────────────────┘
   */

  char         data_type;
  unsigned int width, height, n_channels;

  fread(&data_type, sizeof(char), 1, fin);
  fread(&width, sizeof(unsigned int), 1, fin);
  fread(&height, sizeof(unsigned int), 1, fin);
  fread(&n_channels, sizeof(unsigned int), 1, fin);

  if (n_channels != 1)
  {
    std::cerr << "Unsupported number of channels: " << n_channels << std::endl;
    return -1;
  }

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

  size_t n_elems = width * height;

  float *r_buffer = new float[n_elems];
  float *g_buffer = new float[n_elems];
  float *b_buffer = new float[n_elems];

  memset(r_buffer, 0, n_elems * sizeof(float));
  memset(g_buffer, 0, n_elems * sizeof(float));
  memset(b_buffer, 0, n_elems * sizeof(float));

  float *bayer[4] = {g_buffer, b_buffer, r_buffer, g_buffer};

  if (data_type == 4)   // unsigned short
  {
    unsigned short *buff = new unsigned short[n_elems];
    fread(buff, sizeof(unsigned short), n_elems, fin);

    // Populate each color from the bayered image
    const auto startBayer = std::chrono::high_resolution_clock::now();

#pragma omp parallel for schedule(static)
    for (int y = 0; y < (int)height / 2; y++)
    {
      for (size_t x = 0; x < width / 2; x++)
      {
        // clang-format off
        bayer[0][(2 * y    ) * width + 2 * x    ] = (float)buff[(2 * y    ) * width + 2 * x    ];
        bayer[1][(2 * y    ) * width + 2 * x + 1] = (float)buff[(2 * y    ) * width + 2 * x + 1];
        bayer[2][(2 * y + 1) * width + 2 * x    ] = (float)buff[(2 * y + 1) * width + 2 * x    ];
        bayer[3][(2 * y + 1) * width + 2 * x + 1] = (float)buff[(2 * y + 1) * width + 2 * x + 1];
        // clang-format on
      }
    }

    const auto stopBayer = std::chrono::high_resolution_clock::now();

    delete[] buff;

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

    float *r_cv_buffer = new float[n_elems];
    float *g_cv_buffer = new float[n_elems];
    float *b_cv_buffer = new float[n_elems];

    const auto startConvolve = std::chrono::high_resolution_clock::now();

    convolve3x3(r_matrix, r_buffer, r_cv_buffer, width, height);
    convolve3x3(g_matrix, g_buffer, g_cv_buffer, width, height);
    convolve3x3(b_matrix, b_buffer, b_cv_buffer, width, height);

    const auto stopConvolve = std::chrono::high_resolution_clock::now();


    // Finally, we can write the image
    const auto startWriteImage = std::chrono::high_resolution_clock::now();

    write_image_rgb(filename_out, r_cv_buffer, g_cv_buffer, b_cv_buffer, width, height);

    const auto stopWriteImage = std::chrono::high_resolution_clock::now();

    const auto durationBayer    = std::chrono::duration_cast<std::chrono::milliseconds>(stopBayer - startBayer);
    const auto durationConvolve = std::chrono::duration_cast<std::chrono::milliseconds>(stopConvolve - startConvolve);
    const auto durationWriteImage
        = std::chrono::duration_cast<std::chrono::milliseconds>(stopWriteImage - startWriteImage);

    std::cout << "Bayer:          " << durationBayer.count() << "ms" << std::endl
              << "Convolve:       " << durationConvolve.count() << "ms" << std::endl
              << "Write image:    " << durationWriteImage.count() << "ms" << std::endl;

    delete[] r_cv_buffer;
    delete[] g_cv_buffer;
    delete[] b_cv_buffer;
  }
  else
  {
    std::cerr << "Unsupported data type: " << data_type << std::endl;
  }

  delete[] r_buffer;
  delete[] g_buffer;
  delete[] b_buffer;
}