#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <io.h>
#include <image.h>
#include <imagedng.h>

#include "matrix3x3.h"

int main(int argc, char* argv[])
{
    if (argc < 4) {
        printf(
          "Usage:\n"
          "------\n"
          "raw-to-dng <in_image_raw> <in_correction_matrix> <out_image_dng>\n");

        return 0;
    }

    const char* filename_image_in  = argv[1];
    const char* filename_matrix    = argv[2];
    const char* filename_image_out = argv[3];

    // Allocated during load
    float* bayered_pixels = NULL;

    uint32_t filters;
    size_t   width, height;

    float* matrix = NULL;
    size_t mat_size;

    // ------------------------------------------------------------------------
    // 1 - Load the propriatery RAW image
    // ------------------------------------------------------------------------

    int err = 0;

    const size_t len = strlen(filename_image_in);

    if (
      strcmp(filename_image_in + len - 4, "tiff") == 0 || strcmp(filename_image_in + len - 4, "tiff") == 0
      || strcmp(filename_image_in + len - 3, "tif") == 0 || strcmp(filename_image_in + len - 3, "tif") == 0
      || strcmp(filename_image_in + len - 3, "exr") == 0 || strcmp(filename_image_in + len - 3, "EXR") == 0) {
        float* image_g = NULL;
        float* image_b = NULL;

        err     = read_image_rgb(filename_image_in, &bayered_pixels, &image_g, &image_b, &width, &height);
        filters = 0x49494949;

        free(image_g);
        free(image_b);
    } else if (strcmp(filename_image_in + len - 3, "txt") == 0 || strcmp(filename_image_in + len - 3, "TXT") == 0) {
        err = read_raw_file(filename_image_in, &bayered_pixels, &width, &height, &filters);
    } else {
        fprintf(stderr, "This input file extension is not supported: %s\n", filename_image_in);
        err = -1;
    }

    // Early fail
    if (err != 0) {
        fprintf(stderr, "An error occured when loading the image.\n");
        goto clean;
    }

    // ------------------------------------------------------------------------
    // 2 - Load the matrix
    // ------------------------------------------------------------------------

    err = load_xyz(filename_matrix, &matrix, &mat_size);

    // TODO: this is not ideal: matrix size is not properly tested
    if (err != 0 || mat_size != 3) {
        fprintf(stderr, "An error occured when loading the matrix.\n");
        goto clean;
    }

    // the matrix is RGB_camera -> XYZ, it need to be inversed, DNG expects
    // XYZ -> RGB_camera
    float inv_matrix[9];
    inverse(matrix, inv_matrix);

    // ------------------------------------------------------------------------
    // 3 - Save the DNG
    // ------------------------------------------------------------------------
    write_dng(filename_image_out, bayered_pixels, width, height, filters, inv_matrix);

clean:
    free(matrix);
    free(bayered_pixels);

    return err;
}