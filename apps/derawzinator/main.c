#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <image.h>

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf(
        "Usage:\n"
        "------\n"
        "derawzinator <image_in> <image_out>\n");
    return 0;
  }

  const char *filename_in  = argv[1];
  const char *filename_out = argv[2];

  float *image = NULL;
  size_t width, height;

  int ret = read_image(filename_in, &image, &width, &height);

  if (ret != 0)
  {
    fprintf(stderr, "Could not open file: %s\n", filename_in);
    free(image);
    return ret;
  }

  ret = write_image(filename_out, image, width, height);

  if (ret != 0)
  {
    fprintf(stderr, "Could not write file: %s\n", filename_out);
    free(image);
    return ret;
  }

  free(image);

  return 0;
}