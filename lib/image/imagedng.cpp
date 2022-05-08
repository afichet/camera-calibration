#include <imagedng.h>

#include <tiffio.h>

extern "C"
{
    void get_cfa_pattern(const uint32_t filters, char cfa_pattern[3])
    {
        cfa_pattern[0] = FC(0, 0, filters);
        cfa_pattern[1] = FC(0, 1, filters);
        cfa_pattern[2] = FC(1, 0, filters);
        cfa_pattern[3] = FC(1, 1, filters);
    }


    int write_dng(
      const char*    filename,
      float*         image_data,
      size_t         width,
      size_t         height,
      const uint32_t filters,
      const float    cam_xyz[9])
    {
        const short bayerPatternDimensions[] = {2, 2};
        char        cfa_pattern[4];
        get_cfa_pattern(filters, cfa_pattern);

        // bits per pixel, integer
        int bpp = 8 * sizeof(float);

        // Write the tiff file
        TIFF* tiff_file = TIFFOpen(filename, "w");

        if (!tiff_file) {
            return 0;
        }

        const char dng_version[4]          = {1, 1, 0, 0};
        const char dng_backward_version[4] = {1, 0, 0, 0};

        // File info
        TIFFSetField(tiff_file, TIFFTAG_SUBFILETYPE, 0);
        TIFFSetField(tiff_file, TIFFTAG_MAKE, "True Renderer Corp.");
        TIFFSetField(tiff_file, TIFFTAG_MODEL, "Absolute Zero");
        TIFFSetField(tiff_file, TIFFTAG_UNIQUECAMERAMODEL, "True Renderer Corp.");
        TIFFSetField(tiff_file, TIFFTAG_SOFTWARE, "RAWzinator");
        TIFFSetField(tiff_file, TIFFTAG_DNGVERSION, dng_version);
        TIFFSetField(tiff_file, TIFFTAG_DNGBACKWARDVERSION, dng_backward_version);

        // Image dimensions
        TIFFSetField(tiff_file, TIFFTAG_IMAGEWIDTH, (uint32_t)width);
        TIFFSetField(tiff_file, TIFFTAG_IMAGELENGTH, (uint32_t)height);
        TIFFSetField(tiff_file, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(tiff_file, TIFFTAG_BITSPERSAMPLE, bpp);
        TIFFSetField(tiff_file, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
        TIFFSetField(tiff_file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

        // CFA related information
        TIFFSetField(tiff_file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_CFA);
        TIFFSetField(tiff_file, TIFFTAG_CFAREPEATPATTERNDIM, bayerPatternDimensions);
        TIFFSetField(tiff_file, TIFFTAG_CFAPATTERN, 4, cfa_pattern);

        TIFFSetField(tiff_file, TIFFTAG_COLORMATRIX1, 9, cam_xyz);
        TIFFSetField(tiff_file, TIFFTAG_CALIBRATIONILLUMINANT1, 21);

        // Image data
        for (uint32_t y = 0; y < height; y++) {
            TIFFWriteScanline(tiff_file, &image_data[y * width], y,
                              0);   // this writes a single complete row
        }

        TIFFClose(tiff_file);

        return 1;
    }
}