#include <image.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

//#include <lodepng.h>

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

#ifndef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#  define MAX(a, b) (((a) < (b)) ? (b) : (a))
#endif

#ifndef clamp
#  define clamp(v, _min, _max) (MIN(MAX(v, _min), _max))
#endif

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

        x = width-1;

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
#include <color-converter.h>

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

    delete[] bayered_image;

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


#ifdef HAS_TIFF
#  include <tiffio.h>


  int read_tiff(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    // Open image in
    TIFF *tif_in = TIFFOpen(filename, "r");
    if (tif_in == NULL)
    {
      std::cerr << "Cannot open image file " << filename << std::endl;
      return -1;
    }

    uint32 w, h;
    uint16 bps, spp;
    uint16 config;

    TIFFGetField(tif_in, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif_in, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif_in, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(tif_in, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif_in, TIFFTAG_PLANARCONFIG, &config);

    if (spp <= 0 || spp == 2 || config != PLANARCONFIG_CONTIG)
    {
      std::cerr << "This TIFF format is not supported" << std::endl;
      TIFFClose(tif_in);
      return -1;
    }

    tdata_t buf_in;
    buf_in             = _TIFFmalloc(TIFFScanlineSize(tif_in));
    float *framebuffer = (float *)calloc(3 * w * h, sizeof(float));

    if (spp == 1)
    {
      for (uint32 y = 0; y < h; y++)
      {
        TIFFReadScanline(tif_in, buf_in, y, 0);

        for (uint32 x = 0; x < w; x++)
        {
          for (int c = 0; c < 3; c++)
          {
            switch (bps)
            {
              case 8:
                // 8 bit per channel are assumed sRGB encoded.
                // We linearise to RGB
                framebuffer[3 * (y * w + x) + c] = from_sRGB(((uint8 *)buf_in)[x] / 255.f);
                break;

              case 16:
                framebuffer[3 * (y * w + x) + c] = ((uint16 *)buf_in)[x] / 65535.f;
                break;

              case 32:
                framebuffer[3 * (y * w + x) + c] = ((uint32 *)buf_in)[x] / 4294967295.f;
                break;
            }
          }
        }
      }
    }
    else
    {
      for (uint32 y = 0; y < h; y++)
      {
        TIFFReadScanline(tif_in, buf_in, y, 0);

        for (uint32 x = 0; x < w; x++)
        {
          for (int c = 0; c < 3; c++)
          {
            switch (bps)
            {
              case 8:
                // 8 bit per channel are assumed sRGB encoded.
                // We linearise to RGB
                framebuffer[3 * (y * w + x) + c] = from_sRGB(((uint8 *)buf_in)[spp * x + c] / 255.f);
                break;

              case 16:
                framebuffer[3 * (y * w + x) + c] = ((uint16 *)buf_in)[spp * x + c] / 65535.f;
                break;

              case 32:
                framebuffer[3 * (y * w + x) + c] = ((uint32 *)buf_in)[spp * x + c] / 4294967295.f;
                break;
            }
          }
        }
      }
    }

    _TIFFfree(buf_in);
    TIFFClose(tif_in);

    *width  = w;
    *height = h;
    *pixels = framebuffer;

    return 0;
  }


  int read_tiff_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height)
  {
    // Open image in
    TIFF *tif_in = TIFFOpen(filename, "r");
    if (tif_in == NULL)
    {
      std::cerr << "Cannot open image file " << filename << std::endl;
      return -1;
    }

    uint32 w, h;
    uint16 bps, spp;
    uint16 config;

    TIFFGetField(tif_in, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif_in, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif_in, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(tif_in, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif_in, TIFFTAG_PLANARCONFIG, &config);

    if (spp <= 0 || spp == 2 || config != PLANARCONFIG_CONTIG)
    {
      std::cerr << "This TIFF format is not supported" << std::endl;
      TIFFClose(tif_in);
      return -1;
    }

    tdata_t buf_in;
    buf_in               = _TIFFmalloc(TIFFScanlineSize(tif_in));
    float *framebuffer_r = new float[w * h];
    float *framebuffer_g = new float[w * h];
    float *framebuffer_b = new float[w * h];

    if (spp == 1)
    {
      for (uint32 y = 0; y < h; y++)
      {
        TIFFReadScanline(tif_in, buf_in, y, 0);

        for (uint32 x = 0; x < w; x++)
        {
          float v = 0;
          switch (bps)
          {
            case 8:
              // 8 bit per channel are assumed sRGB encoded.
              // We linearise to RGB
              v = from_sRGB(((uint8 *)buf_in)[x] / 255.f);

              break;

            case 16:
              v = ((uint16 *)buf_in)[x] / 65535.f;
              break;

            case 32:
              v = ((uint32 *)buf_in)[x] / 4294967295.f;
              break;
          }

          framebuffer_r[y * w + x] = v;
          framebuffer_g[y * w + x] = v;
          framebuffer_b[y * w + x] = v;
        }
      }
    }
    else
    {
      for (uint32 y = 0; y < h; y++)
      {
        TIFFReadScanline(tif_in, buf_in, y, 0);

        for (uint32 x = 0; x < w; x++)
        {
          switch (bps)
          {
            case 8:
              // 8 bit per channel are assumed sRGB encoded.
              // We linearise to RGB
              framebuffer_r[y * w + x] = from_sRGB(((uint8 *)buf_in)[spp * x + 0] / 255.f);
              framebuffer_g[y * w + x] = from_sRGB(((uint8 *)buf_in)[spp * x + 1] / 255.f);
              framebuffer_b[y * w + x] = from_sRGB(((uint8 *)buf_in)[spp * x + 2] / 255.f);
              break;

            case 16:
              framebuffer_r[y * w + x] = ((uint16 *)buf_in)[spp * x + 0] / 65535.f;
              framebuffer_g[y * w + x] = ((uint16 *)buf_in)[spp * x + 1] / 65535.f;
              framebuffer_b[y * w + x] = ((uint16 *)buf_in)[spp * x + 2] / 65535.f;
              break;

            case 32:
              framebuffer_r[y * w + x] = ((uint32 *)buf_in)[spp * x + 0] / 4294967295.f;
              framebuffer_g[y * w + x] = ((uint32 *)buf_in)[spp * x + 1] / 4294967295.f;
              framebuffer_b[y * w + x] = ((uint32 *)buf_in)[spp * x + 2] / 4294967295.f;
              break;
          }
        }
      }
    }

    _TIFFfree(buf_in);
    TIFFClose(tif_in);

    *width        = w;
    *height       = h;
    *pixels_red   = framebuffer_r;
    *pixels_green = framebuffer_g;
    *pixels_blue  = framebuffer_b;

    return 0;
  }


  int write_tiff(const char *filename, const float *pixels, uint32 width, uint32 height, uint16 bps)
  {
    // Open image out
    TIFF *tif_out = TIFFOpen(filename, "w");

    if (tif_out == NULL)
    {
      fprintf(stderr, "Cannot open image file\n");
      return -1;
    }

    TIFFSetField(tif_out, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif_out, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif_out, TIFFTAG_BITSPERSAMPLE, bps);
    TIFFSetField(tif_out, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif_out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif_out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

    tdata_t buf_out = _TIFFmalloc(TIFFScanlineSize(tif_out));

    for (uint32 y = 0; y < height; y++)
    {
      for (uint32 x = 0; x < width; x++)
      {
        for (int c = 0; c < 3; c++)
        {
          switch (bps)
          {
            case 8:
              // 8 bit per channel are assumed sRGB encoded.
              ((uint8 *)buf_out)[3 * x + c] = to_sRGB(pixels[3 * (y * width + x) + c]) * 255.f;
              break;

            case 16:
              ((uint16 *)buf_out)[3 * x + c] = clamp(pixels[3 * (y * width + x) + c], 0.f, 1.f) * 65535.f;
              break;

            case 32:
              ((uint32 *)buf_out)[3 * x + c] = clamp(pixels[3 * (y * width + x) + c], 0.f, 1.f) * 4294967295.f;
              break;
          }
        }
      }

      TIFFWriteScanline(tif_out, buf_out, y, 0);
    }

    _TIFFfree(buf_out);
    TIFFClose(tif_out);

    return 0;
  }

  int write_tiff_rgb(
      const char * filename,
      const float *pixels_red,
      const float *pixels_green,
      const float *pixels_blue,
      uint32       width,
      uint32       height,
      uint16       bps)
  {
    // Open image out
    TIFF *tif_out = TIFFOpen(filename, "w");

    if (tif_out == NULL)
    {
      fprintf(stderr, "Cannot open image file\n");
      return -1;
    }

    TIFFSetField(tif_out, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif_out, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif_out, TIFFTAG_BITSPERSAMPLE, bps);
    TIFFSetField(tif_out, TIFFTAG_SAMPLESPERPIXEL, 3);
    TIFFSetField(tif_out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif_out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

    tdata_t buf_out = _TIFFmalloc(TIFFScanlineSize(tif_out));

    for (uint32 y = 0; y < height; y++)
    {
      for (uint32 x = 0; x < width; x++)
      {
        switch (bps)
        {
          case 8:
            // 8 bit per channel are assumed sRGB encoded.
            ((uint8 *)buf_out)[3 * x + 0] = to_sRGB(pixels_red[y * width + x]) * 255.f;
            ((uint8 *)buf_out)[3 * x + 1] = to_sRGB(pixels_green[y * width + x]) * 255.f;
            ((uint8 *)buf_out)[3 * x + 2] = to_sRGB(pixels_blue[y * width + x]) * 255.f;
            break;

          case 16:
            ((uint16 *)buf_out)[3 * x + 0] = clamp(pixels_red[y * width + x], 0.f, 1.f) * 65535.f;
            ((uint16 *)buf_out)[3 * x + 1] = clamp(pixels_green[y * width + x], 0.f, 1.f) * 65535.f;
            ((uint16 *)buf_out)[3 * x + 2] = clamp(pixels_blue[y * width + x], 0.f, 1.f) * 65535.f;
            break;

          case 32:
            ((uint32 *)buf_out)[3 * x + 0] = clamp(pixels_red[y * width + x], 0.f, 1.f) * 4294967295.f;
            ((uint32 *)buf_out)[3 * x + 1] = clamp(pixels_green[y * width + x], 0.f, 1.f) * 4294967295.f;
            ((uint32 *)buf_out)[3 * x + 2] = clamp(pixels_blue[y * width + x], 0.f, 1.f) * 4294967295.f;
            break;
        }
      }

      TIFFWriteScanline(tif_out, buf_out, y, 0);
    }

    _TIFFfree(buf_out);
    TIFFClose(tif_out);

    return 0;
  }
#endif   // HAS_TIFF


  // int read_png(const char *filename, float **pixels, size_t *width, size_t *height)
  // {
  //   // Open image in
  //   unsigned char *image;
  //   unsigned       w, h;

  //   const int error = lodepng_decode32_file(&image, &w, &h, filename);

  //   if (error)
  //   {
  //     fprintf(stderr, "Cannot open image file %s\nError #%u: %s\n", filename, error, lodepng_error_text(error));
  //     return -1;
  //   }

  //   float *buffer = (float *)calloc(3 * w * h, sizeof(float));
  //   for (size_t i = 0; i < w * h; i++)
  //   {
  //     for (int c = 0; c < 3; c++)
  //     {
  //       buffer[3 * i + c] = from_sRGB((float)image[4 * i + c] / 255.f);
  //     }
  //   }

  //   free(image);

  //   *width  = w;
  //   *height = h;
  //   *pixels = buffer;

  //   return 0;
  // }


  // int write_png(const char *filename, const float *pixels, size_t width, size_t height)
  // {
  //   unsigned char *image = (unsigned char *)calloc(4 * width * height, sizeof(unsigned char));

  //   for (size_t i = 0; i < width * height; i++)
  //   {
  //     for (int c = 0; c < 3; c++)
  //     {
  //       image[4 * i + c] = to_sRGB(pixels[3 * i + c]) * 255.f;
  //     }
  //     image[4 * i + 3] = 255;
  //   }

  //   const int error = lodepng_encode32_file(filename, image, width, height);

  //   free(image);

  //   if (error)
  //   {
  //     fprintf(stderr, "Cannot write image file %s\nError #%u: %s\n", filename, error, lodepng_error_text(error));
  //     return -1;
  //   }

  //   return 0;
  // }


  int read_exr(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    EXRVersion version;
    int        status = ParseEXRVersionFromFile(&version, filename);

    if (status != TINYEXR_SUCCESS)
    {
      fprintf(stderr, "Invalid EXR file %s\n", filename);
      return -1;
    }

    if (version.multipart)
    {
      fprintf(stderr, "We do not support EXR multipart\n");
      return -1;
    }

    EXRHeader header;
    InitEXRHeader(&header);

    const char *err = NULL;
    status          = ParseEXRHeaderFromFile(&header, &version, filename, &err);

    if (status != TINYEXR_SUCCESS)
    {
      if (err)
      {
        fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }
      return -1;
    }

    EXRImage image;
    InitEXRImage(&image);

    status = LoadEXRImageFromFile(&image, &header, filename, &err);
    if (status != TINYEXR_SUCCESS)
    {
      if (err)
      {
        fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }
      FreeEXRImage(&image);
      return -1;
    }

    if (image.images == NULL)
    {
      fprintf(stderr, "Load EXR error: Tiled format not supported\n");
      FreeEXRImage(&image);
      return -1;
    }

    int idxR = 0;
    int idxG = 0;
    int idxB = 0;

    for (int c = 0; c < header.num_channels; c++)
    {
      if (strcmp(header.channels[c].name, "R") == 0)
      {
        idxR = c;
      }
      else if (strcmp(header.channels[c].name, "G") == 0)
      {
        idxG = c;
      }
      else if (strcmp(header.channels[c].name, "B") == 0)
      {
        idxB = c;
      }
    }

    float *framebuffer = (float *)calloc(3 * image.width * image.height, sizeof(float));

    if (header.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
    {
      float *buffer_r = (float *)(image.images[idxR]);
      float *buffer_g = (float *)(image.images[idxG]);
      float *buffer_b = (float *)(image.images[idxB]);

#pragma omp parallel for
      for (int i = 0; i < image.width * image.height; i++)
      {
        // set all channels to 0 in case exr file contains only one or two
        // channels
        framebuffer[3 * i + 0] = buffer_r[i];
        framebuffer[3 * i + 1] = buffer_g[i];
        framebuffer[3 * i + 2] = buffer_b[i];
      }
    }
    else if (header.pixel_types[0] == TINYEXR_PIXELTYPE_UINT)
    {
      unsigned int *buffer_r = (unsigned int *)(image.images[idxR]);
      unsigned int *buffer_g = (unsigned int *)(image.images[idxG]);
      unsigned int *buffer_b = (unsigned int *)(image.images[idxB]);

#pragma omp parallel for
      for (int i = 0; i < image.width * image.height; i++)
      {
        // set all channels to 0 in case exr file contains only one or two
        // channels
        framebuffer[3 * i + 0] = buffer_r[i] / std::numeric_limits<unsigned int>::max();
        framebuffer[3 * i + 1] = buffer_g[i] / std::numeric_limits<unsigned int>::max();
        framebuffer[3 * i + 2] = buffer_b[i] / std::numeric_limits<unsigned int>::max();
      }
    }
    else
    {
      FreeEXRImage(&image);
      free(framebuffer);
    }

    // Free image data
    FreeEXRImage(&image);

    *width  = image.width;
    *height = image.height;
    *pixels = framebuffer;

    return status;
  }


  int read_exr_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height)
  {
    EXRVersion version;
    int        status = ParseEXRVersionFromFile(&version, filename);

    if (status != TINYEXR_SUCCESS)
    {
      fprintf(stderr, "Invalid EXR file %s\n", filename);
      return -1;
    }

    if (version.multipart)
    {
      fprintf(stderr, "We do not support EXR multipart\n");
      return -1;
    }

    EXRHeader header;
    InitEXRHeader(&header);

    const char *err = NULL;
    status          = ParseEXRHeaderFromFile(&header, &version, filename, &err);

    if (status != TINYEXR_SUCCESS)
    {
      if (err)
      {
        fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }
      return -1;
    }

    EXRImage image;
    InitEXRImage(&image);

    status = LoadEXRImageFromFile(&image, &header, filename, &err);
    if (status != TINYEXR_SUCCESS)
    {
      if (err)
      {
        fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }
      FreeEXRImage(&image);
      return -1;
    }

    if (image.images == NULL)
    {
      fprintf(stderr, "Load EXR error: Tiled format not supported\n");
      FreeEXRImage(&image);
      return -1;
    }

    int idxR = 0;
    int idxG = 0;
    int idxB = 0;

    for (int c = 0; c < header.num_channels; c++)
    {
      if (strcmp(header.channels[c].name, "R") == 0)
      {
        idxR = c;
      }
      else if (strcmp(header.channels[c].name, "G") == 0)
      {
        idxG = c;
      }
      else if (strcmp(header.channels[c].name, "B") == 0)
      {
        idxB = c;
      }
    }

    float *framebuffer_r = new float[image.width * image.height];
    float *framebuffer_g = new float[image.width * image.height];
    float *framebuffer_b = new float[image.width * image.height];

    if (header.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT)
    {
      float *buffer_r = (float *)(image.images[idxR]);
      float *buffer_g = (float *)(image.images[idxG]);
      float *buffer_b = (float *)(image.images[idxB]);

      memcpy(framebuffer_r, buffer_r, image.width * image.height * sizeof(float));
      memcpy(framebuffer_g, buffer_g, image.width * image.height * sizeof(float));
      memcpy(framebuffer_b, buffer_b, image.width * image.height * sizeof(float));
    }
    else if (header.pixel_types[0] == TINYEXR_PIXELTYPE_UINT)
    {
      unsigned int *buffer_r = (unsigned int *)(image.images[idxR]);
      unsigned int *buffer_g = (unsigned int *)(image.images[idxG]);
      unsigned int *buffer_b = (unsigned int *)(image.images[idxB]);

#pragma omp parallel for
      for (int i = 0; i < image.width * image.height; i++)
      {
        // set all channels to 0 in case exr file contains only one or two
        // channels
        framebuffer_r[i] = buffer_r[i] / std::numeric_limits<unsigned int>::max();
        framebuffer_g[i] = buffer_g[i] / std::numeric_limits<unsigned int>::max();
        framebuffer_b[i] = buffer_b[i] / std::numeric_limits<unsigned int>::max();
      }
    }
    else
    {
      FreeEXRImage(&image);
      delete[] framebuffer_r;
      delete[] framebuffer_g;
      delete[] framebuffer_b;
    }

    // Free image data
    FreeEXRImage(&image);

    *width  = image.width;
    *height = image.height;

    *pixels_red   = framebuffer_r;
    *pixels_green = framebuffer_g;
    *pixels_blue  = framebuffer_b;

    return status;
  }

  int write_exr(const char *filename, const float *pixels, size_t width, size_t height)
  {
    // We need to split R, G, B in separate monochromatic images
    float *red   = new float[width * height];
    float *green = new float[width * height];
    float *blue  = new float[width * height];

#pragma omp parallel for
    for (size_t i = 0; i < width * height; i++)
    {
      red[i]   = pixels[3 * i + 0];
      green[i] = pixels[3 * i + 1];
      blue[i]  = pixels[3 * i + 2];
    }

    int status = write_exr_rgb(filename, red, green, blue, width, height);

    delete[] red;
    delete[] green;
    delete[] blue;

    return status;
  }


  int write_exr_rgb(
      const char * filename,
      const float *pixels_red,
      const float *pixels_green,
      const float *pixels_blue,
      size_t       width,
      size_t       height)
  {
    EXRHeader header;
    EXRImage  image;

    InitEXRHeader(&header);
    InitEXRImage(&image);

    // Must be (A)BGR order, since most of EXR viewers expect this channel order.
    const char * channels     = "BGR";
    const float *image_ptr[3] = {pixels_blue, pixels_green, pixels_red};

    image.num_channels = 3;
    image.width        = width;
    image.height       = height;
    image.images       = (unsigned char **)image_ptr;

    header.num_channels          = image.num_channels;
    header.channels              = new EXRChannelInfo[header.num_channels];
    header.pixel_types           = new int[header.num_channels];
    header.requested_pixel_types = new int[header.num_channels];

    for (int i = 0; i < header.num_channels; i++)
    {
      header.channels[i].name[0]      = channels[i];
      header.channels[i].name[1]      = '\0';
      header.pixel_types[i]           = TINYEXR_PIXELTYPE_FLOAT;   // pixel type of input image
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;   // pixel type of output image to be stored in .EXR
    }

    const char *err    = NULL;
    const int   status = SaveEXRImageToFile(&image, &header, filename, &err);

    if (status != TINYEXR_SUCCESS)
    {
      if (err)
      {
        fprintf(stderr, "Cannot write image file %s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }

      delete[] header.channels;
      delete[] header.pixel_types;
      delete[] header.requested_pixel_types;

      header.channels              = NULL;
      header.pixel_types           = NULL;
      header.requested_pixel_types = NULL;

      FreeEXRHeader(&header);

      return status;
    }

    delete[] header.channels;
    delete[] header.pixel_types;
    delete[] header.requested_pixel_types;

    header.channels              = NULL;
    header.pixel_types           = NULL;
    header.requested_pixel_types = NULL;

    FreeEXRHeader(&header);

    return 0;
  }


  int read_dat(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    float *r_cv_buffer = NULL;
    float *g_cv_buffer = NULL;
    float *b_cv_buffer = NULL;

    int ret = read_dat_rgb(filename, &r_cv_buffer, &g_cv_buffer, &b_cv_buffer, width, height);

    if (ret != 0)
    {
      return ret;
    }

    // Recompose the image
    const size_t size       = (*width) * (*height);
    float *      out_buffer = new float[3 * size];

#pragma omp parallel for
    for (int i = 0; i < int(size); i++)
    {
      out_buffer[3 * i + 0] = r_cv_buffer[i];
      out_buffer[3 * i + 1] = g_cv_buffer[i];
      out_buffer[3 * i + 2] = b_cv_buffer[i];
    }

    delete[] r_cv_buffer;
    delete[] g_cv_buffer;
    delete[] b_cv_buffer;

    *pixels = out_buffer;

    return 0;
  }


  int read_dat_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height)
  {
    FILE *fin = fopen(filename, "rb");

    if (fin == NULL)
    {
      std::cerr << "Cannot open image file " << filename << std::endl;
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
    unsigned int l_width, l_height, n_channels;

    fread(&data_type, sizeof(char), 1, fin);
    fread(&l_width, sizeof(unsigned int), 1, fin);
    fread(&l_height, sizeof(unsigned int), 1, fin);
    fread(&n_channels, sizeof(unsigned int), 1, fin);

    if (n_channels != 1)
    {
      fprintf(stderr, "Unsupported number of channels: %d", n_channels);
    }

    size_t n_elems = l_width * l_height;

    float *bayered_image = new float[n_elems];

    switch (data_type)
    {
      case 2:   // unsigned char
      {
        unsigned char *buff = new unsigned char[n_elems];
        fread(buff, sizeof(unsigned char), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;

      case 3:   // char
      {
        char *buff = new char[n_elems];
        fread(buff, sizeof(char), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;
      case 4:   // unsigned short
      {
        unsigned short *buff = new unsigned short[n_elems];
        fread(buff, sizeof(unsigned short), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;

      case 5:   // short
      {
        short *buff = new short[n_elems];
        fread(buff, sizeof(short), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;

      case 6:   // unsigned int
      {
        unsigned int *buff = new unsigned int[n_elems];
        fread(buff, sizeof(unsigned int), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;

      case 7:   // int
      {
        int *buff = new int[n_elems];
        fread(buff, sizeof(int), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;

      case 8:   // float
      {
        float *buff = new float[n_elems];
        fread(buff, sizeof(float), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;

      case 9:   // double
      {
        double *buff = new double[n_elems];
        fread(buff, sizeof(double), n_elems, fin);

        // Copy cast
#pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
        {
          bayered_image[i] = (float)buff[i];
        }

        delete[] buff;
      }
      break;

      default:
        std::cerr << "Unsupported data type: " << data_type << std::endl;
        return -1;
    }

    *pixels_red   = new float[n_elems];
    *pixels_green = new float[n_elems];
    *pixels_blue  = new float[n_elems];

    basic_debayer(bayered_image, *pixels_red, *pixels_green, *pixels_blue, l_width, l_height);

    *width  = l_width;
    *height = l_height;

    return 0;
  }


  int read_image(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    const size_t len = strlen(filename);

    // if (strcmp(filename + len - 3, "png") == 0 || strcmp(filename + len - 3, "PNG") == 0)  {
    //   return read_png(filename, pixels, width, height);
    // }

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return read_exr(filename, pixels, width, height);
    }

    if (strcmp(filename + len - 3, "dat") == 0 || strcmp(filename + len - 3, "DAT") == 0)
    {
      return read_dat(filename, pixels, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "tiff") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "tif") == 0)
    {
      return read_tiff(filename, pixels, width, height);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }


  int read_image_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height)
  {
    const size_t len = strlen(filename);

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return read_exr_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }

    if (strcmp(filename + len - 3, "dat") == 0 || strcmp(filename + len - 3, "DAT") == 0)
    {
      return read_dat_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "tiff") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "tif") == 0)
    {
      return read_tiff_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }


  int write_image(const char *filename, const float *pixels, size_t width, size_t height)
  {
    const size_t len = strlen(filename);

    // if (strcmp(filename + len - 3, "png") == 0 || strcmp(filename + len - 3, "PNG") == 0)  {
    //   return write_png(filename, pixels, width, height);
    // }

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return write_exr(filename, pixels, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "TIFF") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "TIF") == 0)
    {
      return write_tiff(filename, pixels, width, height, 8);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }


  int write_image_rgb(
      const char * filename,
      const float *pixels_red,
      const float *pixels_green,
      const float *pixels_blue,
      size_t       width,
      size_t       height)
  {
    const size_t len = strlen(filename);

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return write_exr_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "TIFF") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "TIF") == 0)
    {
      return write_tiff_rgb(filename, pixels_red, pixels_green, pixels_blue, width, height, 8);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
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
