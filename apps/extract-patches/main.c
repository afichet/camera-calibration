#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <color-converter.h>
#include <io.h>
#include <image.h>

typedef struct {
    float x, y;
} Point;


typedef struct {
    Point a, b, c, d;
} Box;


int load_boxfile(const char* filename, Box** areas, size_t* size)
{
    FILE* fin = fopen(filename, "r");

    if (fin == NULL) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return -1;
    }

    Box* read_boxes = NULL;
    *size           = 0;

    while (!feof(fin)) {
        (*size)++;
        Box* read_boxes_temp = (Box*)realloc(read_boxes, (*size) * sizeof(Box));

        if (read_boxes_temp == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            fclose(fin);
            free(read_boxes);
            return -1;
        }

        read_boxes = read_boxes_temp;

        int r = fscanf(
          fin,
          "%f,%f;%f,%f;%f,%f;%f,%f;\n",
          &(read_boxes[*size - 1].a.x),
          &(read_boxes[*size - 1].a.y),
          &(read_boxes[*size - 1].b.x),
          &(read_boxes[*size - 1].b.y),
          &(read_boxes[*size - 1].c.x),
          &(read_boxes[*size - 1].c.y),
          &(read_boxes[*size - 1].d.x),
          &(read_boxes[*size - 1].d.y));

        if (r == 0) {
            fprintf(stderr, "Error while reading file %s\n", filename);
            fclose(fin);
            free(read_boxes);
            return -1;
        }
    }

    fclose(fin);

    *areas = read_boxes;

    return 0;
}


int isInTriangle(const Point* p, const Point* p0, const Point* p1, const Point* p2)
{
    const float as_x = p->x - p0->x;
    const float as_y = p->y - p0->y;
    const float s_ab = (p1->x - p0->x) * as_y - (p1->y - p0->y) * as_x > 0;

    if (((p2->x - p0->x) * as_y - (p2->y - p0->y) * as_x > 0) == s_ab) return 0;
    if (((p2->x - p1->x) * (p->y - p1->y) - (p2->y - p1->y) * (p->x - p1->x) > 0) != s_ab) return 0;

    return 1;
}


int isInBox(const Point* p, const Box* b)
{
    if (isInTriangle(p, &b->a, &b->b, &b->c) != 0 || isInTriangle(p, &b->a, &b->c, &b->d) != 0) {
        return 1;
    }

    return 0;
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

    const char* filename_image = argv[1];
    const char* filename_areas = argv[2];
    const char* filename_out   = argv[3];

    float* image = NULL;
    size_t width, height;

    Box*   areas = NULL;
    size_t n_areas;

    float* patches     = NULL;
    float* patches_avg = NULL;

    int err = load_boxfile(filename_areas, &areas, &n_areas);
    if (err != 0) {
        fprintf(stderr, "Cannot open area file %s\n", filename_areas);
        goto clean;
    }

    err = read_image(filename_image, &image, &width, &height);
    if (err != 0) {
        fprintf(stderr, "Cannot read input image file\n");
        goto clean;
    }

    patches = (float*)calloc(4 * n_areas, sizeof(float));
    memset(patches, 0, 4 * n_areas * sizeof(float));

    // Average value computation for each patch
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            const Point currentPixel = {(float)x, (float)y};

            for (size_t patch = 0; patch < n_areas; patch++) {
                if (isInBox(&currentPixel, &areas[patch])) {
                    for (int c = 0; c < 3; c++) {
                        patches[4 * patch + c] += image[3 * (y * width + x) + c];
                    }
                    patches[4 * patch + 3] += 1;

                    break;
                }
            }
        }
    }

    patches_avg = (float*)calloc(3 * n_areas, sizeof(float));

    for (size_t i = 0; i < n_areas; i++) {
        for (int c = 0; c < 3; c++) {
            patches_avg[3 * i + c] = patches[4 * i + c] / patches[4 * i + 3];
        }
    }

    err = save_xyz(filename_out, patches_avg, n_areas);

    if (err != 0) {
        fprintf(stderr, "Cannot save patches file\n");
        goto clean;
    }

clean:
    free(image);
    free(areas);
    free(patches);
    free(patches_avg);

    return err;
}
