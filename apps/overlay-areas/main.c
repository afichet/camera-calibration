#include <stdio.h>
#include <stdlib.h>

#include <io.h>
#include <color-converter.h>
#include <image.h>

typedef struct
{
  float x, y;
} Point;


typedef struct
{
  Point a, b, c, d;
} Box;


int load_boxfile(const char *filename, Box **areas, size_t *size)
{
  FILE *fin = fopen(filename, "r");

  if (fin == NULL)
  {
    fprintf(stderr, "Cannot open file %s\n", filename);
    return -1;
  }

  Box *read_boxes = NULL;
  *size           = 0;

  while (!feof(fin))
  {
    (*size)++;
    Box *read_boxes_temp = (Box *)realloc(read_boxes, (*size) * sizeof(Box));

    if (read_boxes_temp == NULL)
    {
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

    if (r == 0)
    {
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


int isInTriangle(const Point *p, const Point *p0, const Point *p1, const Point *p2)
{
  const float as_x = p->x - p0->x;
  const float as_y = p->y - p0->y;
  const float s_ab = (p1->x - p0->x) * as_y - (p1->y - p0->y) * as_x > 0;

  if (((p2->x - p0->x) * as_y - (p2->y - p0->y) * as_x > 0) == s_ab) return 0;
  if (((p2->x - p1->x) * (p->y - p1->y) - (p2->y - p1->y) * (p->x - p1->x) > 0) != s_ab) return 0;

  return 1;
}


int isInBox(const Point *p, const Box *b)
{
  if (isInTriangle(p, &b->a, &b->b, &b->c) != 0 || isInTriangle(p, &b->a, &b->c, &b->d) != 0)
  {
    return 1;
  }

  return 0;
}


int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    printf(
        "Usage:\n"
        "------\n"
        "overlay-areas <image_in> <areas> <image_out>\n");

    return 0;
  }

  const char *filename_image_in  = argv[1];
  const char *filename_areas     = argv[2];
  const char *filename_image_out = argv[3];

  float *image = NULL;
  size_t width, height;

  Box *  areas = NULL;
  size_t n_areas;

  // Open image in
  int err = read_image(filename_image_in, &image, &width, &height);

  if (err != 0)
  {
    fprintf(stderr, "Cannot read input image file\n");
    return -1;
  }

  // Load areas

  err = load_boxfile(filename_areas, &areas, &n_areas);
  if (err != 0)
  {
    fprintf(stderr, "Cannot open area file %s\n", filename_areas);
    free(image);
    return -1;
  }

  // Overlay
  for (size_t y = 0; y < height; y++)
  {
    for (size_t x = 0; x < width; x++)
    {
      const Point currentPixel = {(float)x, (float)y};

      for (size_t patch = 0; patch < n_areas; patch++)
      {
        if (isInBox(&currentPixel, &areas[patch]) != 0)
        {
          for (int c = 0; c < 3; c++)
          {
            image[3 * (y * width + x) + c] *= .5f;
          }

          break;
        }
      }
    }
  }

  // Image writing
  err = write_image(filename_image_out, image, width, height);

  free(image);
  free(areas);

  if (err != 0)
  {
    fprintf(stderr, "Cannot write output image file\n");
    return -1;
  }

  return 0;
}
