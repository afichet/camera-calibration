#include <stdio.h>
#include <stdlib.h>

#include <tiffio.h>

#include <io.h>
#include <color-converter.h>

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf(
        "Usage:\n"
        "------\n"
        "correct-image <image_in> <matrix> <image_out>\n");

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

  // Load matrix
  float *matrix = NULL;
  size_t mat_size;

  load_xyz(argv[2], &matrix, &mat_size);

  if (matrix == NULL)
  {
    fprintf(stderr, "Cannot open patches or matrix file\n");
    TIFFClose(tif_in);
    free(matrix);
    return -1;
  }


  // Open image out
  TIFF *tif_out = TIFFOpen(argv[3], "w");
  if (tif_out == NULL)
  {
    fprintf(stderr, "Cannot open image file\n");
    TIFFClose(tif_in);
    free(matrix);
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

  for (uint32 y = 0; y < h; y++)
  {
    TIFFReadScanline(tif_in, buf_in, y, 0);
    float color_xyz[3];
    float color_xyz_corrected[3];
    float color_rgb_corrected[3];

    for (uint32 x = 0; x < w; x++)
    {
      for (int c = 0; c < 3; c++)
      {
        switch (bps)
        {
          case 8:
            // 8 bit per channel are assumed sRGB encoded.
            // We linearise to RGB
            color_xyz[c] = from_sRGB(((uint8 *)buf_in)[spp * x + c] / 255.f);
            break;

          case 16:
            color_xyz[c] = ((uint16 *)buf_in)[spp * x + c] / 65535.f;
            break;

          case 32:
            color_xyz[c] = ((uint32 *)buf_in)[spp * x + c] / 4294967295.f;
            break;
        }
      }

      matmul(matrix, color_xyz, color_xyz_corrected);
      XYZ_to_RGB(color_xyz_corrected, color_rgb_corrected);

      for (int c = 0; c < 3; c++)
      {
        switch (bps)
        {
          case 8:
            // 8 bit per channel are assumed sRGB encoded.
            ((uint8 *)buf_out)[spp * x + c] = to_sRGB(color_rgb_corrected[c]) * 255.f;
            break;

          case 16:
            ((uint16 *)buf_out)[spp * x + c] = color_rgb_corrected[c] * 65535.f;
            break;

          case 32:
            ((uint32 *)buf_out)[spp * x + c] = color_rgb_corrected[c] * 4294967295.f;
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

  free(matrix);


  return 0;
}
