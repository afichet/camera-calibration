#include <image.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

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

extern "C"
{
#include <color-converter.h>

#ifdef HAS_TIFF
#  include <tiffio.h>

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
        fprintf(stderr, "Cannot write image file %s\nError: %s\n", filename, err);
        FreeEXRErrorMessage(err);
      }

      return -1;
    }

    return 0;
  }


  int read_dat(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    FILE *fin = fopen(filename, "rb");

    if (fin == NULL)
    {
      fprintf(stderr, "Cannot open image file %s", filename);
      return -1;
    }

    // Image format
    // ------------
    //
    // | Type         | Description              |
    // | ------------ | ------------------------ |
    // | char         | datatype                 |
    // | unsigned int | width                    |
    // | unsigned int | height                   |
    // | unsigned int | number of channers       |
    // | void*        | image data               |

    // Data types
    // ----------
    //
    // | Value | Type           |
    // | ----- | -------------- |
    // | 1     | BOOL           |
    // | 2     | unsigned char  |
    // | 3     | char           |
    // | 4     | unsigned short |
    // | 5     | short          |
    // | 6     | unsigned int   |
    // | 7     | int            |
    // | 8     | float          |
    // | 9     | double         |

    char         data_type;
    unsigned int l_width, l_height, n_channels;
    void *       in_raw_buff = NULL;

    fread(&data_type, sizeof(char), 1, fin);
    fread(&l_width, sizeof(unsigned int), 1, fin);
    fread(&l_height, sizeof(unsigned int), 1, fin);
    fread(&n_channels, sizeof(unsigned int), 1, fin);

    if (n_channels != 1 && n_channels != 3)
    {
      fprintf(stderr, "Unsupported number of channels: %d", n_channels);
    }

    size_t n_elems    = l_width * l_height * n_channels;
    float *out_buffer = (float *)calloc(3 * l_width * l_height, sizeof(float));

    switch (data_type)
    {
      case 0:
        fprintf(stderr, "Undefined data type\n");
        free(out_buffer);
        return -1;

      case 1:   // BOOL
        fprintf(stderr, "Unsupported data type\n");
        free(out_buffer);
        return -1;

      case 2:   // unsigned char
      {
        in_raw_buff = calloc(n_elems, sizeof(unsigned char));
        fread(in_raw_buff, sizeof(unsigned char), n_elems, fin);

        const unsigned char *buff = (unsigned char *)in_raw_buff;

        if (n_channels == 1)
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            float val = float(buff[i]) / 255.f;
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = val;
            }
          }
        }
        else
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = float(buff[3 * i + c]) / 255.f;
            }
          }
        }

        free(in_raw_buff);
      }
      break;

      case 3:   // char
      {
        in_raw_buff = calloc(n_elems, sizeof(char));
        fread(in_raw_buff, sizeof(char), n_elems, fin);

        const char *buff = (char *)in_raw_buff;

        if (n_channels == 1)
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            float val = float(buff[i]) / 127.f;
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = val;
            }
          }
        }
        else
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = float(buff[3 * i + c]) / 127.f;
            }
          }
        }

        free(in_raw_buff);
      }
      break;
      case 4:   // unsigned short
      {
        in_raw_buff = calloc(n_elems, sizeof(unsigned short));
        fread(in_raw_buff, sizeof(unsigned short), n_elems, fin);

        const unsigned short *buff = (unsigned short *)in_raw_buff;

        if (n_channels == 1)
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            float val = float(buff[i]) / 65535.f;
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = val;
            }
          }
        }
        else
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = float(buff[3 * i + c]) / 65535.f;
            }
          }
        }

        free(in_raw_buff);
      }
      break;

      case 5:   // short
      {
        in_raw_buff = calloc(n_elems, sizeof(short));
        fread(in_raw_buff, sizeof(short), n_elems, fin);

        const short *buff = (short *)in_raw_buff;

        if (n_channels == 1)
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            float val = float(buff[i]) / 32767.f;
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = val;
            }
          }
        }
        else
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = float(buff[3 * i + c]) / 32767.f;
            }
          }
        }

        free(in_raw_buff);
      }
      break;

      case 6:   // unsigned int
      {
        in_raw_buff = calloc(n_elems, sizeof(unsigned int));
        fread(in_raw_buff, sizeof(unsigned int), n_elems, fin);

        const unsigned int *buff = (unsigned int *)in_raw_buff;

        if (n_channels == 1)
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            float val = float(buff[i]) / 65535.f;
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = val;
            }
          }
        }
        else
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = float(buff[3 * i + c]) / 65535.f;
            }
          }
        }

        free(in_raw_buff);
      }
      break;

      case 7:   // int
      {
        in_raw_buff = calloc(n_elems, sizeof(int));
        fread(in_raw_buff, sizeof(int), n_elems, fin);

        const int *buff = (int *)in_raw_buff;

        if (n_channels == 1)
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            float val = float(buff[i]) / 32767.f;
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = val;
            }
          }
        }
        else
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = float(buff[3 * i + c]) / 32767.f;
            }
          }
        }

        free(in_raw_buff);
      }
      break;

      case 8:   // float
      {
        if (n_channels == 1)
        {
          in_raw_buff = calloc(n_elems, sizeof(float));
          fread(in_raw_buff, sizeof(float), n_elems, fin);

          const float *buff = (float *)in_raw_buff;

          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = buff[i];
            }
          }

          free(in_raw_buff);
        }
        else
        {
          // Native type, we do not need to perform extra step
          fread(out_buffer, sizeof(float), 3 * l_width * l_height, fin);
        }
      }
      break;

      case 9:   // double
      {
        in_raw_buff = calloc(n_elems, sizeof(double));
        fread(in_raw_buff, sizeof(double), n_elems, fin);

        const double *buff = (double *)in_raw_buff;

        if (n_channels == 1)
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = buff[i];
            }
          }
        }
        else
        {
          for (size_t i = 0; i < l_width * l_height; i++)
          {
            for (int c = 0; c < 3; c++)
            {
              out_buffer[3 * i + c] = buff[3 * i + c];
            }
          }
        }

        free(in_raw_buff);
      }
      break;

      default:
        fprintf(stderr, "Unsupported data type\n");
        free(out_buffer);
        return -1;
    }

    *pixels = out_buffer;
    *width  = l_width;
    *height = l_height;

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
    if (strcmp(filename + len - 4, "tiff") == 0 || strcmp(filename + len - 4, "TIFF") == 0
        || strcmp(filename + len - 3, "tif") == 0 || strcmp(filename + len - 3, "TIF") == 0)
    {
      return write_tiff(filename, pixels, width, height, 8);
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
