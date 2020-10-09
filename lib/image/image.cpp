
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <lodepng.h>

extern "C"
{
#include <color-converter.h>

#ifdef HAS_TIFF
#  include <tiffio.h>
#endif
}

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

#ifndef min
#  define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#  define max(a, b) (((a) < (b)) ? (b) : (a))
#endif

#ifndef clamp
#  define clamp(v, _min, _max) (min(max(v, _min), _max))
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#ifdef HAS_TIFF
  int read_tiff(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    // Open image in
    TIFF *tif_in = TIFFOpen(filename, "r");
    if (tif_in == NULL)
    {
      fprintf(stderr, "Cannot open image file\n");
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

    if (spp < 3 || config != PLANARCONFIG_CONTIG)
    {
      fprintf(stderr, "Not enough channels in this image\n");
      TIFFClose(tif_in);
      return -1;
    }

    tdata_t buf_in;
    buf_in             = _TIFFmalloc(TIFFScanlineSize(tif_in));
    float *framebuffer = (float *)calloc(3 * w * h, sizeof(float));

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

    _TIFFfree(buf_in);
    TIFFClose(tif_in);

    *width  = w;
    *height = h;
    *pixels = framebuffer;

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

#endif


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
    const char *err = NULL;
    int         w, h;

    const int status = LoadEXRWithLayer(pixels, &w, &h, filename, NULL, &err);

    if (status != TINYEXR_SUCCESS)
    {
      if (err)
      {
        fprintf(stderr, "Cannot read image file%s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }

      return -1;
    }

    *width  = w;
    *height = h;

    return 0;
  }


  int write_exr(const char *filename, const float *pixels, size_t width, size_t height)
  {
    const char *err = NULL;

    const int status = SaveEXR(pixels, width, height, 3, 0, filename, &err);

    if (status != TINYEXR_SUCCESS)
    {
      if (err)
      {
        fprintf(stderr, "Cannot write image file%s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }

      return -1;
    }

    return 0;
  }


  int read_image(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    size_t len = strlen(filename);

    // if (strcmp(filename + len - 3, "png") == 0 || strcmp(filename + len - 3, "PNG") == 0)  {
    //   return read_png(filename, pixels, width, height);
    // }

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return read_exr(filename, pixels, width, height);
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


  int write_image(const char *filename, const float *pixels, size_t width, size_t height)
  {
    size_t len = strlen(filename);

    // if (strcmp(filename + len - 3, "png") == 0 || strcmp(filename + len - 3, "PNG") == 0)  {
    //   return write_png(filename, pixels, width, height);
    // }

    if (strcmp(filename + len - 3, "exr") == 0 || strcmp(filename + len - 3, "EXR") == 0)
    {
      return write_exr(filename, pixels, width, height);
    }

#ifdef HAS_TIFF
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "tiff") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "tif") == 0)
    {
      return write_tiff(filename, pixels, width, height, 8);
    }
#endif

    fprintf(stderr, "This extension is not supported %s\n", filename);

    return -1;
  }

#ifdef __cplusplus
}
#endif
