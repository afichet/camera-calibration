#include <stdio.h>
#include <stdlib.h>

#include <io.h>
#include <color-converter.h>

int main(int argc, char* argv[])
{
    if (argc < 4) {
        printf(
          "Usage:\n"
          "------\n"
          "correct-patches <data_xyz> <matrix> <out_data_xyz>\n");

        return 0;
    }

    const char* filename_patches_in  = argv[1];
    const char* filename_matrix      = argv[2];
    const char* filename_patches_out = argv[3];

    float* patches_in = NULL;
    float* matrix     = NULL;
    size_t size       = 0;
    size_t mat_size;

    int err = load_xyz(filename_patches_in, &patches_in, &size);

    if (err != 0) {
        fprintf(stderr, "Cannot open input patches file\n");
        return -1;
    }

    err = load_xyz(filename_matrix, &matrix, &mat_size);

    if (err != 0 || mat_size != 3) {
        fprintf(stderr, "Cannot open matrix file\n");
        free(patches_in);
        return -1;
    }

    float* patches_out = (float*)calloc(3 * size, sizeof(float));

    // We multiply the correction matrix with each patch color
    for (size_t i = 0; i < size; i++) {
        matmul(matrix, &patches_in[3 * i], &patches_out[3 * i]);
    }

    err = save_xyz(filename_patches_out, patches_out, size);

    free(patches_in);
    free(matrix);
    free(patches_out);

    if (err != 0) {
        fprintf(stderr, "Cannot write output patches file\n");
        return -1;
    }

    return 0;
}
