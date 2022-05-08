#include <imagergb.h>

#include <iostream>

#include <color-converter.h>


#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>


#ifndef MIN
#    define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#    define MAX(a, b) (((a) < (b)) ? (b) : (a))
#endif

#ifndef clamp
#    define clamp(v, _min, _max) (MIN(MAX(v, _min), _max))
#endif

extern "C"
{
#ifdef HAS_TIFF
#    include <tiffio.h>

    int read_tiff(const char* filename, float** pixels, size_t* width, size_t* height)
    {
        // Open image in
        TIFF* tif_in = TIFFOpen(filename, "r");
        if (tif_in == NULL) {
            std::cerr << "Cannot open image file " << filename << std::endl;
            return -1;
        }

        uint32_t w, h;
        uint16_t bps, spp;
        uint16_t config;

        TIFFGetField(tif_in, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif_in, TIFFTAG_IMAGELENGTH, &h);
        TIFFGetField(tif_in, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif_in, TIFFTAG_SAMPLESPERPIXEL, &spp);
        TIFFGetField(tif_in, TIFFTAG_PLANARCONFIG, &config);

        if (spp <= 0 || spp == 2 || config != PLANARCONFIG_CONTIG) {
            std::cerr << "This TIFF format is not supported" << std::endl;
            TIFFClose(tif_in);
            return -1;
        }

        tdata_t buf_in;
        buf_in             = _TIFFmalloc(TIFFScanlineSize(tif_in));
        float* framebuffer = (float*)calloc(3 * w * h, sizeof(float));

        if (spp == 1) {
            for (uint32_t y = 0; y < h; y++) {
                TIFFReadScanline(tif_in, buf_in, y, 0);

                for (uint32_t x = 0; x < w; x++) {
                    for (int c = 0; c < 3; c++) {
                        switch (bps) {
                            case 8:
                                // 8 bit per channel are assumed sRGB encoded.
                                // We linearise to RGB
                                framebuffer[3 * (y * w + x) + c] = from_sRGB(((uint8_t*)buf_in)[x] / 255.f);
                                break;

                            case 16:
                                framebuffer[3 * (y * w + x) + c] = ((uint16_t*)buf_in)[x] / 65535.f;
                                break;

                            case 32:
                                framebuffer[3 * (y * w + x) + c] = ((uint32_t*)buf_in)[x] / 4294967295.f;
                                break;
                        }
                    }
                }
            }
        } else {
            for (uint32_t y = 0; y < h; y++) {
                TIFFReadScanline(tif_in, buf_in, y, 0);

                for (uint32_t x = 0; x < w; x++) {
                    for (int c = 0; c < 3; c++) {
                        switch (bps) {
                            case 8:
                                // 8 bit per channel are assumed sRGB encoded.
                                // We linearise to RGB
                                framebuffer[3 * (y * w + x) + c] = from_sRGB(((uint8_t*)buf_in)[spp * x + c] / 255.f);
                                break;

                            case 16:
                                framebuffer[3 * (y * w + x) + c] = ((uint16_t*)buf_in)[spp * x + c] / 65535.f;
                                break;

                            case 32:
                                framebuffer[3 * (y * w + x) + c] = ((uint32_t*)buf_in)[spp * x + c] / 4294967295.f;
                                break;
                        }
                    }
                }
            }
        }

        _TIFFfree(buf_in);
        TIFFClose(tif_in);

        *width  = w;
        *height = h;
        *pixels = framebuffer;

        return 0;
    }


    int read_tiff_rgb(
      const char* filename,
      float**     pixels_red,
      float**     pixels_green,
      float**     pixels_blue,
      size_t*     width,
      size_t*     height)
    {
        // Open image in
        TIFF* tif_in = TIFFOpen(filename, "r");
        if (tif_in == NULL) {
            std::cerr << "Cannot open image file " << filename << std::endl;
            return -1;
        }

        uint32_t w, h;
        uint16_t bps, spp;
        uint16_t config;

        TIFFGetField(tif_in, TIFFTAG_IMAGEWIDTH, &w);
        TIFFGetField(tif_in, TIFFTAG_IMAGELENGTH, &h);
        TIFFGetField(tif_in, TIFFTAG_BITSPERSAMPLE, &bps);
        TIFFGetField(tif_in, TIFFTAG_SAMPLESPERPIXEL, &spp);
        TIFFGetField(tif_in, TIFFTAG_PLANARCONFIG, &config);

        if (spp <= 0 || spp == 2 || config != PLANARCONFIG_CONTIG) {
            std::cerr << "This TIFF format is not supported" << std::endl;
            TIFFClose(tif_in);
            return -1;
        }

        tdata_t buf_in;
        buf_in               = _TIFFmalloc(TIFFScanlineSize(tif_in));
        float* framebuffer_r = new float[w * h];
        float* framebuffer_g = new float[w * h];
        float* framebuffer_b = new float[w * h];

        if (spp == 1) {
            for (uint32_t y = 0; y < h; y++) {
                TIFFReadScanline(tif_in, buf_in, y, 0);

                for (uint32_t x = 0; x < w; x++) {
                    float v = 0;
                    switch (bps) {
                        case 8:
                            // 8 bit per channel are assumed sRGB encoded.
                            // We linearise to RGB
                            v = from_sRGB(((uint8_t*)buf_in)[x] / 255.f);

                            break;

                        case 16:
                            v = ((uint16_t*)buf_in)[x] / 65535.f;
                            break;

                        case 32:
                            v = ((uint32_t*)buf_in)[x] / 4294967295.f;
                            break;
                    }

                    framebuffer_r[y * w + x] = v;
                    framebuffer_g[y * w + x] = v;
                    framebuffer_b[y * w + x] = v;
                }
            }
        } else {
            for (uint32_t y = 0; y < h; y++) {
                TIFFReadScanline(tif_in, buf_in, y, 0);

                for (uint32_t x = 0; x < w; x++) {
                    switch (bps) {
                        case 8:
                            // 8 bit per channel are assumed sRGB encoded.
                            // We linearise to RGB
                            framebuffer_r[y * w + x] = from_sRGB(((uint8_t*)buf_in)[spp * x + 0] / 255.f);
                            framebuffer_g[y * w + x] = from_sRGB(((uint8_t*)buf_in)[spp * x + 1] / 255.f);
                            framebuffer_b[y * w + x] = from_sRGB(((uint8_t*)buf_in)[spp * x + 2] / 255.f);
                            break;

                        case 16:
                            framebuffer_r[y * w + x] = ((uint16_t*)buf_in)[spp * x + 0] / 65535.f;
                            framebuffer_g[y * w + x] = ((uint16_t*)buf_in)[spp * x + 1] / 65535.f;
                            framebuffer_b[y * w + x] = ((uint16_t*)buf_in)[spp * x + 2] / 65535.f;
                            break;

                        case 32:
                            framebuffer_r[y * w + x] = ((uint32_t*)buf_in)[spp * x + 0] / 4294967295.f;
                            framebuffer_g[y * w + x] = ((uint32_t*)buf_in)[spp * x + 1] / 4294967295.f;
                            framebuffer_b[y * w + x] = ((uint32_t*)buf_in)[spp * x + 2] / 4294967295.f;
                            break;
                    }
                }
            }
        }

        _TIFFfree(buf_in);
        TIFFClose(tif_in);

        *width        = w;
        *height       = h;
        *pixels_red   = framebuffer_r;
        *pixels_green = framebuffer_g;
        *pixels_blue  = framebuffer_b;

        return 0;
    }


    int write_tiff(const char* filename, const float* pixels, uint32_t width, uint32_t height, uint16_t bps)
    {
        // Open image out
        TIFF* tif_out = TIFFOpen(filename, "w");

        if (tif_out == NULL) {
            fprintf(stderr, "Cannot open image file\n");
            return -1;
        }

        TIFFSetField(tif_out, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif_out, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif_out, TIFFTAG_BITSPERSAMPLE, bps);
        TIFFSetField(tif_out, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(tif_out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif_out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

        tdata_t buf_out = _TIFFmalloc(TIFFScanlineSize(tif_out));

        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                for (int c = 0; c < 3; c++) {
                    switch (bps) {
                        case 8:
                            // 8 bit per channel are assumed sRGB encoded.
                            ((uint8_t*)buf_out)[3 * x + c] = to_sRGB(pixels[3 * (y * width + x) + c]) * 255.f;
                            break;

                        case 16:
                            ((uint16_t*)buf_out)[3 * x + c]
                              = clamp(pixels[3 * (y * width + x) + c], 0.f, 1.f) * 65535.f;
                            break;

                        case 32:
                            ((uint32_t*)buf_out)[3 * x + c]
                              = clamp(pixels[3 * (y * width + x) + c], 0.f, 1.f) * 4294967295.f;
                            break;
                    }
                }
            }

            TIFFWriteScanline(tif_out, buf_out, y, 0);
        }

        _TIFFfree(buf_out);
        TIFFClose(tif_out);

        return 0;
    }

    int write_tiff_rgb(
      const char*  filename,
      const float* pixels_red,
      const float* pixels_green,
      const float* pixels_blue,
      uint32_t     width,
      uint32_t     height,
      uint16_t     bps)
    {
        // Open image out
        TIFF* tif_out = TIFFOpen(filename, "w");

        if (tif_out == NULL) {
            fprintf(stderr, "Cannot open image file\n");
            return -1;
        }

        TIFFSetField(tif_out, TIFFTAG_IMAGEWIDTH, width);
        TIFFSetField(tif_out, TIFFTAG_IMAGELENGTH, height);
        TIFFSetField(tif_out, TIFFTAG_BITSPERSAMPLE, bps);
        TIFFSetField(tif_out, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(tif_out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(tif_out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);

        tdata_t buf_out = _TIFFmalloc(TIFFScanlineSize(tif_out));

        for (uint32_t y = 0; y < height; y++) {
            for (uint32_t x = 0; x < width; x++) {
                switch (bps) {
                    case 8:
                        // 8 bit per channel are assumed sRGB encoded.
                        ((uint8_t*)buf_out)[3 * x + 0] = to_sRGB(pixels_red[y * width + x]) * 255.f;
                        ((uint8_t*)buf_out)[3 * x + 1] = to_sRGB(pixels_green[y * width + x]) * 255.f;
                        ((uint8_t*)buf_out)[3 * x + 2] = to_sRGB(pixels_blue[y * width + x]) * 255.f;
                        break;

                    case 16:
                        ((uint16_t*)buf_out)[3 * x + 0] = clamp(pixels_red[y * width + x], 0.f, 1.f) * 65535.f;
                        ((uint16_t*)buf_out)[3 * x + 1] = clamp(pixels_green[y * width + x], 0.f, 1.f) * 65535.f;
                        ((uint16_t*)buf_out)[3 * x + 2] = clamp(pixels_blue[y * width + x], 0.f, 1.f) * 65535.f;
                        break;

                    case 32:
                        ((uint32_t*)buf_out)[3 * x + 0] = clamp(pixels_red[y * width + x], 0.f, 1.f) * 4294967295.f;
                        ((uint32_t*)buf_out)[3 * x + 1] = clamp(pixels_green[y * width + x], 0.f, 1.f) * 4294967295.f;
                        ((uint32_t*)buf_out)[3 * x + 2] = clamp(pixels_blue[y * width + x], 0.f, 1.f) * 4294967295.f;
                        break;
                }
            }

            TIFFWriteScanline(tif_out, buf_out, y, 0);
        }

        _TIFFfree(buf_out);
        TIFFClose(tif_out);

        return 0;
    }
#endif   // HAS_TIFF


    int read_exr(const char* filename, float** pixels, size_t* width, size_t* height)
    {
        EXRVersion version;
        int        status = ParseEXRVersionFromFile(&version, filename);

        if (status != TINYEXR_SUCCESS) {
            fprintf(stderr, "Invalid EXR file %s\n", filename);
            return -1;
        }

        if (version.multipart) {
            fprintf(stderr, "We do not support EXR multipart\n");
            return -1;
        }

        EXRHeader header;
        InitEXRHeader(&header);

        const char* err = NULL;
        status          = ParseEXRHeaderFromFile(&header, &version, filename, &err);

        if (status != TINYEXR_SUCCESS) {
            if (err) {
                fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
                FreeEXRErrorMessage(err);
            }
            return -1;
        }

        EXRImage image;
        InitEXRImage(&image);

        status = LoadEXRImageFromFile(&image, &header, filename, &err);
        if (status != TINYEXR_SUCCESS) {
            if (err) {
                fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
                FreeEXRErrorMessage(err);
            }
            FreeEXRImage(&image);
            return -1;
        }

        if (image.images == NULL) {
            fprintf(stderr, "Load EXR error: Tiled format not supported\n");
            FreeEXRImage(&image);
            return -1;
        }

        int idxR = 0;
        int idxG = 0;
        int idxB = 0;

        for (int c = 0; c < header.num_channels; c++) {
            if (strcmp(header.channels[c].name, "R") == 0) {
                idxR = c;
            } else if (strcmp(header.channels[c].name, "G") == 0) {
                idxG = c;
            } else if (strcmp(header.channels[c].name, "B") == 0) {
                idxB = c;
            }
        }

        float* framebuffer = (float*)calloc(3 * image.width * image.height, sizeof(float));

        if (header.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT) {
            float* buffer_r = (float*)(image.images[idxR]);
            float* buffer_g = (float*)(image.images[idxG]);
            float* buffer_b = (float*)(image.images[idxB]);

            #pragma omp parallel for
            for (int i = 0; i < image.width * image.height; i++) {
                // set all channels to 0 in case exr file contains only one or two
                // channels
                framebuffer[3 * i + 0] = buffer_r[i];
                framebuffer[3 * i + 1] = buffer_g[i];
                framebuffer[3 * i + 2] = buffer_b[i];
            }
        } else if (header.pixel_types[0] == TINYEXR_PIXELTYPE_UINT) {
            unsigned int* buffer_r = (unsigned int*)(image.images[idxR]);
            unsigned int* buffer_g = (unsigned int*)(image.images[idxG]);
            unsigned int* buffer_b = (unsigned int*)(image.images[idxB]);

            #pragma omp parallel for
            for (int i = 0; i < image.width * image.height; i++) {
                // set all channels to 0 in case exr file contains only one or two
                // channels
                framebuffer[3 * i + 0] = buffer_r[i] / std::numeric_limits<unsigned int>::max();
                framebuffer[3 * i + 1] = buffer_g[i] / std::numeric_limits<unsigned int>::max();
                framebuffer[3 * i + 2] = buffer_b[i] / std::numeric_limits<unsigned int>::max();
            }
        } else {
            FreeEXRImage(&image);
            free(framebuffer);
        }

        // Free image data
        FreeEXRImage(&image);

        *width  = image.width;
        *height = image.height;
        *pixels = framebuffer;

        return status;
    }


    int read_exr_rgb(
      const char* filename,
      float**     pixels_red,
      float**     pixels_green,
      float**     pixels_blue,
      size_t*     width,
      size_t*     height)
    {
        EXRVersion version;
        int        status = ParseEXRVersionFromFile(&version, filename);

        if (status != TINYEXR_SUCCESS) {
            fprintf(stderr, "Invalid EXR file %s\n", filename);
            return -1;
        }

        if (version.multipart) {
            fprintf(stderr, "We do not support EXR multipart\n");
            return -1;
        }

        EXRHeader header;
        InitEXRHeader(&header);

        const char* err = NULL;
        status          = ParseEXRHeaderFromFile(&header, &version, filename, &err);

        if (status != TINYEXR_SUCCESS) {
            if (err) {
                fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
                FreeEXRErrorMessage(err);
            }
            return -1;
        }

        EXRImage image;
        InitEXRImage(&image);

        status = LoadEXRImageFromFile(&image, &header, filename, &err);
        if (status != TINYEXR_SUCCESS) {
            if (err) {
                fprintf(stderr, "Cannot open EXR file %s\nError: %s\n", filename, err);
                FreeEXRErrorMessage(err);
            }
            FreeEXRImage(&image);
            return -1;
        }

        if (image.images == NULL) {
            fprintf(stderr, "Load EXR error: Tiled format not supported\n");
            FreeEXRImage(&image);
            return -1;
        }

        int idxR = 0;
        int idxG = 0;
        int idxB = 0;

        for (int c = 0; c < header.num_channels; c++) {
            if (strcmp(header.channels[c].name, "R") == 0) {
                idxR = c;
            } else if (strcmp(header.channels[c].name, "G") == 0) {
                idxG = c;
            } else if (strcmp(header.channels[c].name, "B") == 0) {
                idxB = c;
            }
        }

        float* framebuffer_r = new float[image.width * image.height];
        float* framebuffer_g = new float[image.width * image.height];
        float* framebuffer_b = new float[image.width * image.height];

        if (header.pixel_types[0] == TINYEXR_PIXELTYPE_FLOAT) {
            float* buffer_r = (float*)(image.images[idxR]);
            float* buffer_g = (float*)(image.images[idxG]);
            float* buffer_b = (float*)(image.images[idxB]);

            memcpy(framebuffer_r, buffer_r, image.width * image.height * sizeof(float));
            memcpy(framebuffer_g, buffer_g, image.width * image.height * sizeof(float));
            memcpy(framebuffer_b, buffer_b, image.width * image.height * sizeof(float));
        } else if (header.pixel_types[0] == TINYEXR_PIXELTYPE_UINT) {
            unsigned int* buffer_r = (unsigned int*)(image.images[idxR]);
            unsigned int* buffer_g = (unsigned int*)(image.images[idxG]);
            unsigned int* buffer_b = (unsigned int*)(image.images[idxB]);

            #pragma omp parallel for
            for (int i = 0; i < image.width * image.height; i++) {
                // set all channels to 0 in case exr file contains only one or two
                // channels
                framebuffer_r[i] = buffer_r[i] / std::numeric_limits<unsigned int>::max();
                framebuffer_g[i] = buffer_g[i] / std::numeric_limits<unsigned int>::max();
                framebuffer_b[i] = buffer_b[i] / std::numeric_limits<unsigned int>::max();
            }
        } else {
            FreeEXRImage(&image);
            delete[] framebuffer_r;
            delete[] framebuffer_g;
            delete[] framebuffer_b;
        }

        // Free image data
        FreeEXRImage(&image);

        *width  = image.width;
        *height = image.height;

        *pixels_red   = framebuffer_r;
        *pixels_green = framebuffer_g;
        *pixels_blue  = framebuffer_b;

        return status;
    }


    int write_exr(const char* filename, const float* pixels, size_t width, size_t height)
    {
        // We need to split R, G, B in separate monochromatic images
        float* red   = new float[width * height];
        float* green = new float[width * height];
        float* blue  = new float[width * height];

        #pragma omp parallel for
        for (size_t i = 0; i < width * height; i++) {
            red[i]   = pixels[3 * i + 0];
            green[i] = pixels[3 * i + 1];
            blue[i]  = pixels[3 * i + 2];
        }

        int status = write_exr_rgb(filename, red, green, blue, width, height);

        delete[] red;
        delete[] green;
        delete[] blue;

        return status;
    }


    int write_exr_rgb(
      const char*  filename,
      const float* pixels_red,
      const float* pixels_green,
      const float* pixels_blue,
      size_t       width,
      size_t       height)
    {
        EXRHeader header;
        EXRImage  image;

        InitEXRHeader(&header);
        InitEXRImage(&image);

        // Must be (A)BGR order, since most of EXR viewers expect this channel order.
        const char*  channels     = "BGR";
        const float* image_ptr[3] = {pixels_blue, pixels_green, pixels_red};

        image.num_channels = 3;
        image.width        = width;
        image.height       = height;
        image.images       = (unsigned char**)image_ptr;

        header.num_channels          = image.num_channels;
        header.channels              = new EXRChannelInfo[header.num_channels];
        header.pixel_types           = new int[header.num_channels];
        header.requested_pixel_types = new int[header.num_channels];

        for (int i = 0; i < header.num_channels; i++) {
            header.channels[i].name[0] = channels[i];
            header.channels[i].name[1] = '\0';
            header.pixel_types[i]      = TINYEXR_PIXELTYPE_FLOAT;   // pixel type of input image
            header.requested_pixel_types[i]
              = TINYEXR_PIXELTYPE_FLOAT;   // pixel type of output image to be stored in .EXR
        }

        const char* err    = NULL;
        const int   status = SaveEXRImageToFile(&image, &header, filename, &err);

        if (status != TINYEXR_SUCCESS) {
            if (err) {
                fprintf(stderr, "Cannot write image file %s\nError: %s\n", filename, err);
                FreeEXRErrorMessage(err);
            }

            delete[] header.channels;
            delete[] header.pixel_types;
            delete[] header.requested_pixel_types;

            header.channels              = NULL;
            header.pixel_types           = NULL;
            header.requested_pixel_types = NULL;

            FreeEXRHeader(&header);

            return status;
        }

        delete[] header.channels;
        delete[] header.pixel_types;
        delete[] header.requested_pixel_types;

        header.channels              = NULL;
        header.pixel_types           = NULL;
        header.requested_pixel_types = NULL;

        FreeEXRHeader(&header);

        return 0;
    }
}