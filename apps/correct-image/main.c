#include <stdio.h>
#include <stdlib.h>

#include <image.h>

#include <io.h>
#include <color-converter.h>

int main(int argc, char* argv[])
{
    if (argc < 4) {
        printf(
          "Usage:\n"
          "------\n"
          "correct-image <image_in> <matrix> <image_out>\n");

        return 0;
    }

    const char* filename_image_in  = argv[1];
    const char* filename_matrix    = argv[2];
    const char* filename_image_out = argv[3];

    float* image  = NULL;
    float* matrix = NULL;
    size_t width, height;
    size_t mat_size;

    int err = load_xyz(filename_matrix, &matrix, &mat_size);

    if (err != 0 || mat_size != 3) {
        fprintf(stderr, "Cannot open matrix file\n");
        return -1;
    }

    err = read_image(filename_image_in, &image, &width, &height);

    if (err != 0) {
        fprintf(stderr, "Cannot read input image file\n");
        free(matrix);
        return -1;
    }

    correct_image(image, width, height, matrix);

    err = write_image(filename_image_out, image, width, height);

    free(image);
    free(matrix);

    if (err != 0) {
        fprintf(stderr, "Cannot write output image file\n");
        return -1;
    }

    return 0;
}
