#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <lodepng.h>

#include <io.h>
#include <color-converter.h>


int create_image(float *patches_xyz, size_t width, size_t height, const char *output_file)
{
  const size_t   pixel_buffer_size = 4 * width * height * sizeof(unsigned char);
  unsigned char *pixel_buffer      = (unsigned char *)malloc(pixel_buffer_size);

  if (pixel_buffer == NULL)
  {
    fprintf(stderr, "Memomry allocation error");
    return -1;
  }

  memset(pixel_buffer, 0, pixel_buffer_size);

  for (size_t y = 0; y < height; y++)
  {
    const float v_idx = 4.F * (float)y / (float)height;
    const float f_v   = v_idx - floorf(v_idx);

    for (size_t x = 0; x < width; x++)
    {
      const float u_idx = 6.F * (float)x / (float)width;
      const float f_u   = u_idx - floorf(u_idx);

      const float s_v = 0.1;
      const float s_u = 0.1;

      int idx = (int)u_idx + 6 * (int)v_idx;

      if ((((int)(v_idx) == 0 && f_v > s_v && f_v < 1. - s_v / 2.)
           || ((int)(v_idx) > 0 && (int)(v_idx) < 3 && f_v > s_v / 2. && f_v < 1. - s_v / 2.)
           || ((int)(v_idx) == 3 && f_v > s_v / 2. && f_v < 1. - s_v))
          && (((int)(u_idx) == 0 && f_u > s_u && f_u < 1. - s_u / 2.)
              || ((int)(u_idx) > 0 && (int)(u_idx) < 5 && f_u > s_u / 2. && f_u < 1. - s_u / 2.)
              || ((int)(u_idx) == 5 && f_u > s_u / 2. && f_u < 1. - s_u)))
      {
        float rgb[3];
        XYZ_to_RGB(&patches_xyz[3 * idx], rgb);

        for (int c = 0; c < 3; c++)
        {
          pixel_buffer[4 * (width * y + x) + c] = (unsigned char)(255.f * to_sRGB(rgb[c]));
        }
      }

      pixel_buffer[4 * (width * y + x) + 3] = 255;
    }
  }

  lodepng_encode_file(output_file, pixel_buffer, width, height, LCT_RGBA, 8);

  return 0;
}


int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf(
        "Usage:\n"
        "------\n"
        "gen-colorchart-image <data_xyz> <output_png>\n");

    return 0;
  }

  float *macbeth_patches_xyz = NULL;
  size_t size                = 0;

  load_xyz(argv[1], &macbeth_patches_xyz, &size);
  create_image(macbeth_patches_xyz, 600, 400, argv[2]);

  free(macbeth_patches_xyz);

  return 0;
}
