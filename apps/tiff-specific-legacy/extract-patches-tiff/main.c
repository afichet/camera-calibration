#include <stdio.h>
#include <stdlib.h>

#include <tiffio.h>

#include <color-converter.h>
#include <io.h>

void load_boxfile(const char* filename, int** areas, size_t* size)
{
    FILE* fin = fopen(filename, "r");

    if (fin == NULL) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return;
    }

    int* buff_values = NULL;
    *size            = 0;

    while (!feof(fin)) {
        (*size)++;
        int* buff_values_temp = (int*)realloc(buff_values, 4 * (*size) * sizeof(int));

        if (buff_values_temp == NULL) {
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

        if (r == 0) {
            fprintf(stderr, "Error while reading file %s\n", filename);
        }
    }

    fclose(fin);

    *areas = buff_values;
}


int main(int argc, char* argv[])
{
    if (argc < 4) {
        printf(
          "Usage:\n"
          "------\n"
          "extract-patches <tiff_file> <areas> <output_csv>\n");

        return 0;
    }

    TIFF* tif = TIFFOpen(argv[1], "r");
    if (tif == NULL) {
        fprintf(stderr, "Cannot open image file\n");
        return -1;
    }

    uint32 w, h;
    uint16 bps, spp;
    uint16 config;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
    TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bps);
    TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &spp);
    TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

    if (spp < 3 || config != PLANARCONFIG_CONTIG) {
        TIFFClose(tif);
        fprintf(stderr, "Not enough channels in this image\n");
        return -1;
    }

    int*   areas = NULL;
    size_t n_areas;
    load_boxfile(argv[2], &areas, &n_areas);

    if (areas == NULL) {
        TIFFClose(tif);
        fprintf(stderr, "Cannot open area file\n");
        return -1;
    }

    tdata_t buf;

    buf = _TIFFmalloc(TIFFScanlineSize(tif));

    float* patches = (float*)calloc(3 * n_areas, sizeof(float));

    for (size_t a = 0; a < n_areas; a++) {
        float patch_value[3] = {0};

        // Average value computation for each patch
        for (uint32 y = areas[4 * a + 1]; y < (uint32)areas[4 * a + 3]; y++) {
            TIFFReadScanline(tif, buf, y, 0);
            for (uint32 x = areas[4 * a]; x < (uint32)areas[4 * a + 2]; x++) {
                switch (bps) {
                    case 8:
                        for (int c = 0; c < 3; c++) {
                            // 8 bit per channel are assumed sRGB encoded.
                            // We linearise to RGB
                            patch_value[c] += from_sRGB(((uint8*)buf)[spp * x + c] / 255.f);
                        }
                        break;

                    case 16:
                        for (int c = 0; c < 3; c++) {
                            patch_value[c] += ((uint16*)buf)[spp * x + c] / 65535.f;
                        }

                        break;

                    case 32:
                        for (int c = 0; c < 3; c++) {
                            patch_value[c] += ((uint32*)buf)[spp * x + c] / 4294967295.f;
                        }

                        break;
                }
            }
        }

        const float div = (areas[4 * a + 2] - areas[4 * a]) * (areas[4 * a + 3] - areas[4 * a + 1]);

        for (int i = 0; i < 3; i++) {
            patches[3 * a + i] = patch_value[i] / div;
        }
    }

    _TIFFfree(buf);
    TIFFClose(tif);

    free(areas);

    save_xyz(argv[3], patches, n_areas);

    free(patches);

    return 0;
}