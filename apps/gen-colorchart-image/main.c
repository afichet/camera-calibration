#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <io.h>
#include <color-converter.h>
#include <image.h>


void create_image(float* patches_xyz, size_t width, size_t height, float* framebuffer, int isXYZ)
{
    for (size_t y = 0; y < height; y++) {
        const float v_idx = 4.F * (float)y / (float)height;
        const float f_v   = v_idx - floorf(v_idx);

        for (size_t x = 0; x < width; x++) {
            const float u_idx = 6.F * (float)x / (float)width;
            const float f_u   = u_idx - floorf(u_idx);

            const float s_v = 0.1;
            const float s_u = 0.1;

            int idx = (int)u_idx + 6 * (int)v_idx;

            // We select the patch to use depending on the location on the image
            if (
              (((int)(v_idx) == 0 && f_v > s_v && f_v < 1. - s_v / 2.)
               || ((int)(v_idx) > 0 && (int)(v_idx) < 3 && f_v > s_v / 2. && f_v < 1. - s_v / 2.)
               || ((int)(v_idx) == 3 && f_v > s_v / 2. && f_v < 1. - s_v))
              && (((int)(u_idx) == 0 && f_u > s_u && f_u < 1. - s_u / 2.) || ((int)(u_idx) > 0 && (int)(u_idx) < 5 && f_u > s_u / 2. && f_u < 1. - s_u / 2.) || ((int)(u_idx) == 5 && f_u > s_u / 2. && f_u < 1. - s_u))) {
                if (isXYZ != 0) {
                    float rgb[3];
                    XYZ_to_RGB(&patches_xyz[3 * idx], rgb);
                    memcpy(&framebuffer[3 * (width * y + x)], rgb, 3 * sizeof(float));
                } else {
                    memcpy(&framebuffer[3 * (width * y + x)], &patches_xyz[3 * idx], 3 * sizeof(float));
                }
            } else {
                memset(&framebuffer[3 * (width * y + x)], 0, 3 * sizeof(float));
            }
        }
    }
}


int main(int argc, char* argv[])
{
    if (argc < 3) {
        printf(
          "Usage:\n"
          "------\n"
          "gen-colorchart-image <data_xyz> <output_image> [<isrgb = true>]\n");

        return 0;
    }

    const char* filename_patches_in   = argv[1];
    const char* filename_output_image = argv[2];

    int isXYZ = 1;
    if (argc > 3) {
        isXYZ = strcmp(argv[3], "true");
    }

    const size_t width  = 600;
    const size_t height = 400;

    float* macbeth_patches_xyz = NULL;
    size_t size                = 0;

    int res = load_xyz(filename_patches_in, &macbeth_patches_xyz, &size);

    if (res != 0) {
        fprintf(stderr, "Cannot open the patches file\n");
        return -1;
    }

    float* pixels = (float*)calloc(3 * width * height, sizeof(float));

    create_image(macbeth_patches_xyz, width, height, pixels, isXYZ);

    res = write_image(filename_output_image, pixels, width, height);
    if (res != 0) {
        fprintf(stderr, "Cannot write output file %s\n", filename_output_image);
    }

    free(macbeth_patches_xyz);
    free(pixels);

    return 0;
}
