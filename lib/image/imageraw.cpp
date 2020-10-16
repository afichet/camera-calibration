#include <imageraw.h>
#include <imageprocessing.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

extern "C"
{
  int read_dat(const char *filename, float **pixels, size_t *width, size_t *height)
  {
    float *r_cv_buffer = NULL;
    float *g_cv_buffer = NULL;
    float *b_cv_buffer = NULL;

    int ret = read_dat_rgb(filename, &r_cv_buffer, &g_cv_buffer, &b_cv_buffer, width, height);

    if (ret != 0)
    {
      return ret;
    }

    // Recompose the image
    const size_t size       = (*width) * (*height);
    float *      out_buffer = new float[3 * size];

    #pragma omp parallel for
    for (int i = 0; i < int(size); i++)
    {
      out_buffer[3 * i + 0] = r_cv_buffer[i];
      out_buffer[3 * i + 1] = g_cv_buffer[i];
      out_buffer[3 * i + 2] = b_cv_buffer[i];
    }

    delete[] r_cv_buffer;
    delete[] g_cv_buffer;
    delete[] b_cv_buffer;

    *pixels = out_buffer;

    return 0;
  }

  int read_dat_rgb(
      const char *filename,
      float **    pixels_red,
      float **    pixels_green,
      float **    pixels_blue,
      size_t *    width,
      size_t *    height)
  {
    FILE *fin = fopen(filename, "rb");

    if (fin == NULL)
    {
      std::cerr << "Cannot open image file " << filename << std::endl;
      return -1;
    }

    /**
     * Image format
     * ============
     *
     * File content
     * ------------
     *
     * ┌──────────────┬──────────────────────────┐
     * │ Type         │ Description              │
     * ├──────────────┼──────────────────────────┤
     * │ char         │ datatype                 │
     * │ unsigned int │ height                   │
     * │ unsigned int │ width                    │
     * │ unsigned int │ number of channers       │
     * │ void*        │ image data               │
     * └──────────────┴──────────────────────────┘
     *
     * Data types
     * ----------
     *
     * ┌───────┬────────────────┐
     * │ Value │ Type           │
     * ├───────┼────────────────┤
     * │ 1     │ BOOL           │
     * │ 2     │ unsigned char  │
     * │ 3     │ char           │
     * │ 4     │ unsigned short │
     * │ 5     │ short          │
     * │ 6     │ unsigned int   │
     * │ 7     │ int            │
     * │ 8     │ float          │
     * │ 9     │ double         │
     * └───────┴────────────────┘
     */

    // We read the header
    char         data_type;
    unsigned int l_width, l_height, n_channels;

    int ret_read = 0;

    ret_read += fread(&data_type, sizeof(char), 1, fin);
    ret_read += fread(&l_width, sizeof(unsigned int), 1, fin);
    ret_read += fread(&l_height, sizeof(unsigned int), 1, fin);
    ret_read += fread(&n_channels, sizeof(unsigned int), 1, fin);

    if (ret_read != 4)
    {
      std::cerr << "Incorrect formed header" << std::endl;
      fclose(fin);
      return -1;
    }

    if (n_channels != 1)
    {
      std::cerr << "Unexpeted number of channels: " << n_channels << std::endl;
      fclose(fin);
      return -1;
    }

    // We determine the number of bytes to read based on the datatype
    size_t n_bytes_per_channel;

    switch (data_type)
    {
      case 1:   // bool
        n_bytes_per_channel = sizeof(bool);
        break;

      case 2:   // unsigned char
        n_bytes_per_channel = sizeof(unsigned char);
        break;

      case 3:   // char
        n_bytes_per_channel = sizeof(char);
        break;

      case 4:   // unsigned short
        n_bytes_per_channel = sizeof(unsigned short);
        break;

      case 5:   // short
        n_bytes_per_channel = sizeof(short);
        break;

      case 6:   // unsigned int
        n_bytes_per_channel = sizeof(unsigned int);
        break;

      case 7:   // int
        n_bytes_per_channel = sizeof(int);
        break;

      case 8:   // float
        n_bytes_per_channel = sizeof(float);
        break;

      case 9:   // double
        n_bytes_per_channel = sizeof(double);
        break;

      default:
        std::cerr << "Unsupported data type: " << data_type << std::endl;
        return -1;
    }

    const size_t n_elems = l_width * l_height * n_channels;
    size_t       read_elems;

    // Now read the pixels from the file
    void *read_buff = (void *)malloc(n_bytes_per_channel * n_elems);
    read_elems      = fread(read_buff, n_bytes_per_channel, n_elems, fin);
    fclose(fin);

    if (read_elems != n_elems)
    {
      std::cerr << "The file is corrupted!" << std::endl
                << " - expected pixels: " << n_elems << std::endl
                << " - read pixels:     " << read_elems << std::endl;

      free(read_buff);
      return -1;
    }

    // Copy cast
    float *bayered_image = new float[n_elems];

    switch (data_type)
    {
      case 1:   // bool
      {
        bool *buff = reinterpret_cast<bool *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      case 2:   // unsigned char
      {
        unsigned char *buff = reinterpret_cast<unsigned char *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      case 3:   // char
      {
        char *buff = reinterpret_cast<char *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;
      case 4:   // unsigned short
      {
        unsigned short *buff = reinterpret_cast<unsigned short *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      case 5:   // short
      {
        short *buff = reinterpret_cast<short *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      case 6:   // unsigned int
      {
        unsigned int *buff = reinterpret_cast<unsigned int *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      case 7:   // int
      {
        int *buff = reinterpret_cast<int *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      case 8:   // float
      {
        float *buff = reinterpret_cast<float *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      case 9:   // double
      {
        double *buff = reinterpret_cast<double *>(read_buff);
        #pragma omp parallel for
        for (int i = 0; i < int(n_elems); i++)
          bayered_image[i] = (float)buff[i];
      }
      break;

      default:
        // This should not happen since this is already checked.
        std::cerr << "Unsupported data type: " << data_type << std::endl;
        free(read_buff);
        return -1;
    }

    free(read_buff);

    *pixels_red   = (float *)malloc(sizeof(float) * n_elems);
    *pixels_green = (float *)malloc(sizeof(float) * n_elems);
    *pixels_blue  = (float *)malloc(sizeof(float) * n_elems);

    basic_debayer(bayered_image, *pixels_red, *pixels_green, *pixels_blue, l_width, l_height);

    *width  = l_width;
    *height = l_height;

    delete[] bayered_image;

    return 0;
  }
}