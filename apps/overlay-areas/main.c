#include <stdio.h>
#include <stdlib.h>

#include <tiffio.h>

#include <io.h>
#include <color-converter.h>


void load_boxfile(const char *filename, int **areas, size_t *size)
{
  FILE *fin = fopen(filename, "r");

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open file %s\n", filename);
    return;
  }

  int *buff_values = NULL;
  *size            = 0;

  while (!feof(fin))
  {
    (*size)++;
    int *buff_values_temp = (int *)realloc(buff_values, 4 * (*size) * sizeof(int));

    if (buff_values_temp == NULL)
    {
      fprintf(stderr, "Memory allocation error\n");
      free(buff_values);
      return;
    }

    buff_values = buff_values_temp;

    int r = fscanf(
        fin,
        "%d,%d,%d,%d\n",
        &buff_values[4 * (*size - 1)],
        &buff_values[4 * (*size - 1) + 1],
        &buff_values[4 * (*size - 1) + 2],
        &buff_values[4 * (*size - 1) + 3]);

    if (r == 0)
    {
      fprintf(stderr, "Error while reading file %s\n", filename);
    }
  }

  fclose(fin);

  *areas = buff_values;
}



int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf(
        "Usage:\n"
        "------\n"
        "overlay-areas <iamge_in> <areas> <image_out>\n");

    return 0;
  }

  // Open image in
  TIFF *tif_in = TIFFOpen(argv[1], "r");
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

  // Load areas
  int *  areas = NULL;
  size_t n_areas;
  load_boxfile(argv[2], &areas, &n_areas);

  if (areas == NULL)
  {
    TIFFClose(tif_in);
    fprintf(stderr, "Cannot open area file\n");
    return -1;
  }


  // Open image out
  TIFF *tif_out = TIFFOpen(argv[3], "w");
  if (tif_out == NULL)
  {
    fprintf(stderr, "Cannot open image file\n");
    TIFFClose(tif_in);
    free(areas);
    TIFFClose(tif_out);
    return -1;
  }

  TIFFSetField(tif_out, TIFFTAG_IMAGEWIDTH, w);
  TIFFSetField(tif_out, TIFFTAG_IMAGELENGTH, h);
  TIFFSetField(tif_out, TIFFTAG_BITSPERSAMPLE, bps);
  TIFFSetField(tif_out, TIFFTAG_SAMPLESPERPIXEL, spp);
  TIFFSetField(tif_out, TIFFTAG_PLANARCONFIG, config);
  TIFFSetField(tif_out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

  tdata_t buf_in, buf_out;
  buf_in  = _TIFFmalloc(TIFFScanlineSize(tif_in));
  buf_out = _TIFFmalloc(TIFFScanlineSize(tif_out));

  float *pixel_buffer = (float *)calloc(3 * w * h, sizeof(float));

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
            pixel_buffer[3 * (y * w + x) + c] = ((uint8 *)buf_in)[spp * x + c] / 255.f;
            break;

          case 16:
            pixel_buffer[3 * (y * w + x) + c] = ((uint16 *)buf_in)[spp * x + c] / 65535.f;
            break;

          case 32:
            pixel_buffer[3 * (y * w + x) + c] = ((uint32 *)buf_in)[spp * x + c] / 4294967295.f;
            break;
        }
      }
    }
  }

  // Overlay
  for (size_t a = 0; a < n_areas; a++)
  {
    for (uint32 y = areas[4 * a + 1]; y < (uint32)areas[4 * a + 3]; y++)
    {
      for (uint32 x = areas[4 * a]; x < (uint32)areas[4 * a + 2]; x++)
      {
        for (int c = 1; c < 3; c++) {
          pixel_buffer[3 * (y * w + x) + c] *= .5f;
        }
      }
    }
  }

  // Image writing
  for (uint32 y = 0; y < h; y++)
  {
    for (uint32 x = 0; x < w; x++)
    {
      for (int c = 0; c < 3; c++)
      {
        switch (bps)
        {
          case 8:
            // 8 bit per channel are assumed sRGB encoded.
            ((uint8 *)buf_out)[spp * x + c] = pixel_buffer[3 * (y * w + x) + c] * 255.f;
            break;

          case 16:
            ((uint16 *)buf_out)[spp * x + c] = pixel_buffer[3 * (y * w + x) + c] * 65535.f;
            break;

          case 32:
            ((uint32 *)buf_out)[spp * x + c] = pixel_buffer[3 * (y * w + x) + c] * 4294967295.f;
            break;
        }
      }
    }

    TIFFWriteScanline(tif_out, buf_out, y, 0);
  }

  _TIFFfree(buf_in);
  _TIFFfree(buf_out);
  TIFFClose(tif_in);
  TIFFClose(tif_out);

  free(areas);


  return 0;
}
