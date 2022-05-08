#include <imageraw.h>
#include <imageprocessing.h>
#include <imagergb.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <tinyxml2.h>

extern "C"
{
    int read_raw_metadata(const char* filename, RAWMetadata* metadata)
    {
        tinyxml2::XMLDocument doc;
        tinyxml2::XMLError    err = doc.LoadFile(filename);

        if (err != tinyxml2::XML_SUCCESS) {
            return err;
        }

        tinyxml2::XMLElement* cameraImage = doc.FirstChildElement("CameraImage");

        if (cameraImage == NULL) {
            std::cerr << "Could not read metadata from " << filename << std::endl;
            return -1;
        }

        tinyxml2::XMLElement* lighting    = cameraImage->FirstChildElement("Lighting");
        tinyxml2::XMLElement* cameraState = cameraImage->FirstChildElement("CameraInternalState");

        if (lighting == NULL || cameraState == NULL) {
            std::cerr << "Could not read metadata from " << filename << std::endl;
            return -1;
        }

        tinyxml2::XMLElement* exposureTime  = cameraState->FirstChildElement("exposureTime");
        tinyxml2::XMLElement* aperture      = cameraState->FirstChildElement("aperture");
        tinyxml2::XMLElement* gain          = cameraState->FirstChildElement("gain");
        tinyxml2::XMLElement* bitDepth      = cameraState->FirstChildElement("bitDepth");
        tinyxml2::XMLElement* bayerPattern  = cameraState->FirstChildElement("bayerPattern");
        tinyxml2::XMLElement* filepath_img  = cameraImage->FirstChildElement("filepath_img");
        tinyxml2::XMLElement* filepath_info = cameraImage->FirstChildElement("filepath_info");

        if (
          aperture == NULL || exposureTime == NULL || gain == NULL || bitDepth == NULL || bayerPattern == NULL
          || filepath_img == NULL || filepath_info == NULL) {
            std::cerr << "Could not read metadata from " << filename << std::endl;
            return -1;
        }

        tinyxml2::XMLElement* LED     = lighting->FirstChildElement("LED");
        tinyxml2::XMLElement* LED_idx = NULL;

        if (LED != NULL) {
            LED_idx = LED->FirstChildElement("idx");
        }

        if (LED_idx != NULL) {
            metadata->ledIdx = std::stoi(LED_idx->GetText());
        } else {
            metadata->ledIdx = -1;
        }

        metadata->exposureTime = std::stof(exposureTime->GetText());
        metadata->aperture     = std::stof(aperture->GetText());
        metadata->gain         = std::stof(gain->GetText());
        metadata->bitDepth     = std::stoi(bitDepth->GetText());

        const char* bayer = bayerPattern->GetText();
        if (strlen(bayer) != 4) {
            std::cerr << "Incorrect bayer pattern" << std::endl;
            return -1;
        }

        const char* img_name  = filepath_img->GetText();
        const char* info_name = filepath_info->GetText();

        metadata->bayerPattern = (char*)calloc(5, sizeof(char));
        memcpy(metadata->bayerPattern, bayer, 4 * sizeof(char));
        metadata->bayerPattern[4] = '\0';

        metadata->filename_image = (char*)calloc(strlen(img_name) + 1, sizeof(char));
        metadata->filename_info  = (char*)calloc(strlen(info_name) + 1, sizeof(char));
        strcpy(metadata->filename_image, img_name);
        strcpy(metadata->filename_info, info_name);

        return 0;
    }

    int
    read_raw_file(const char* filename, float** bayered_pixels, size_t* width, size_t* height, unsigned int* filters)
    {
        RAWMetadata metadata;
        metadata.bayerPattern   = NULL;
        metadata.filename_image = NULL;
        metadata.filename_info  = NULL;

        int err = read_raw_metadata(filename, &metadata);

        if (err != 0) {
            free(metadata.bayerPattern);
            free(metadata.filename_image);
            free(metadata.filename_info);

            return err;
        }

        // Now, we need location of the image data
        const size_t len_filename_info = strlen(metadata.filename_info);
        const size_t len_filename_img  = strlen(metadata.filename_image);
        const size_t len_path          = strlen(filename);
        const size_t len_basepath      = len_path - len_filename_info;
        const size_t len_path_img      = len_basepath + len_filename_img;
        char*        path_filename_img = (char*)calloc(len_path_img + 1, sizeof(char));

        // Add the basedir
        strncpy(path_filename_img, filename, len_basepath);
        // Append the image file name
        strcpy(&path_filename_img[len_basepath], metadata.filename_image);

        // Now, we have to use the relevant function depending on the file type
        // This is similar to the read_image_* functions except we have to renormalize
        // data for DAT files

        if (
          strcmp(metadata.filename_image + len_filename_img - 3, "exr") == 0
          || strcmp(metadata.filename_image + len_filename_img - 3, "EXR") == 0) {
            float* pg = NULL;
            float* pb = NULL;
            err       = read_exr_rgb(path_filename_img, bayered_pixels, &pg, &pb, width, height);
            free(pg);
            free(pb);
        } else if (
          strcmp(metadata.filename_image + len_filename_img - 3, "dat") == 0
          || strcmp(metadata.filename_image + len_filename_img - 3, "DAT") == 0) {
            err = read_dat(path_filename_img, bayered_pixels, width, height, metadata.bitDepth);
        }
#ifdef HAS_TIFF
        else if (
          strcmp(metadata.filename_image + len_filename_img - 3, "tif") == 0
          || strcmp(metadata.filename_image + len_filename_img - 3, "TIF") == 0
          || strcmp(metadata.filename_image + len_filename_img - 4, "tiff") == 0
          || strcmp(metadata.filename_image + len_filename_img - 4, "TIFF") == 0) {
            float* pg = NULL;
            float* pb = NULL;
            err       = read_tiff_rgb(path_filename_img, bayered_pixels, &pg, &pb, width, height);
            free(pg);
            free(pb);
        }
#endif
        else {
            err = -1;
        }

        if (err != 0) {
            std::cerr << "Could not decode the image data." << std::endl;
            free(*bayered_pixels);
        }

        if (strcmp(metadata.bayerPattern, "BGGR") == 0) {
            *filters = 0x16161616;
        } else if (strcmp(metadata.bayerPattern, "GRBG") == 0) {
            *filters = 0x61616161;
        } else if (strcmp(metadata.bayerPattern, "GBRG") == 0) {
            *filters = 0x49494949;
        } else if (strcmp(metadata.bayerPattern, "RGGB") == 0) {
            *filters = 0x94949494;
        }

        free(metadata.bayerPattern);
        free(metadata.filename_image);
        free(metadata.filename_info);
        free(path_filename_img);

        return err;
    }



    int read_dat(const char* filename, float** bayered_pixels, size_t* width, size_t* height, size_t bit_depth)
    {
        FILE* fin = fopen(filename, "rb");

        if (fin == NULL) {
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

        if (ret_read != 4) {
            std::cerr << "Incorrect formed header" << std::endl;
            fclose(fin);
            return -1;
        }

        if (n_channels != 1) {
            std::cerr << "Unexpeted number of channels: " << n_channels << std::endl;
            fclose(fin);
            return -1;
        }

        // We determine the number of bytes to read based on the datatype
        size_t n_bytes_per_channel;

        switch (data_type) {
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
        void* read_buff = (void*)malloc(n_bytes_per_channel * n_elems);
        read_elems      = fread(read_buff, n_bytes_per_channel, n_elems, fin);
        fclose(fin);

        if (read_elems != n_elems) {
            std::cerr << "The file is corrupted!" << std::endl
                      << " - expected pixels: " << n_elems << std::endl
                      << " - read pixels:     " << read_elems << std::endl;

            free(read_buff);
            return -1;
        }

        // Copy cast
        float* bayered_image = new float[n_elems];
        float  renorm        = float(1 << bit_depth) - 1.f;

        switch (data_type) {
            case 1:   // bool
            {
                bool* buff = reinterpret_cast<bool*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            case 2:   // unsigned char
            {
                unsigned char* buff = reinterpret_cast<unsigned char*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            case 3:   // char
            {
                char* buff = reinterpret_cast<char*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;
            case 4:   // unsigned short
            {
                unsigned short* buff = reinterpret_cast<unsigned short*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            case 5:   // short
            {
                short* buff = reinterpret_cast<short*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            case 6:   // unsigned int
            {
                unsigned int* buff = reinterpret_cast<unsigned int*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            case 7:   // int
            {
                int* buff = reinterpret_cast<int*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            case 8:   // float
            {
                float* buff = reinterpret_cast<float*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            case 9:   // double
            {
                double* buff = reinterpret_cast<double*>(read_buff);
                #pragma omp parallel for
                for (int i = 0; i < int(n_elems); i++)
                    bayered_image[i] = (float)buff[i] / renorm;
            } break;

            default:
                // This should not happen since this is already checked.
                std::cerr << "Unsupported data type: " << data_type << std::endl;
                free(read_buff);
                return -1;
        }

        free(read_buff);

        *width          = l_width;
        *height         = l_height;
        *bayered_pixels = bayered_image;

        return 0;
    }


    int read_raw(const char* filename, float** pixels, size_t* width, size_t* height, RAWDemosaicMethod method)
    {
        float*       bayered_pixels = NULL;
        unsigned int filters        = 0;

        int ret = read_raw_file(filename, &bayered_pixels, width, height, &filters);

        if (ret != 0) {
            return ret;
        }

        const size_t image_size = (*width) * (*height);

        *pixels = (float*)calloc(3 * image_size, sizeof(float));

        demosaic(bayered_pixels, *pixels, *width, *height, filters, method);

        free(bayered_pixels);

        return 0;
    }


    int read_raw_rgb(
      const char*       filename,
      float**           pixels_red,
      float**           pixels_green,
      float**           pixels_blue,
      size_t*           width,
      size_t*           height,
      RAWDemosaicMethod method)
    {
        float*       bayered_pixels = NULL;
        unsigned int filters        = 0;

        int ret = read_raw_file(filename, &bayered_pixels, width, height, &filters);

        if (ret != 0) {
            return ret;
        }

        const size_t image_size = (*width) * (*height);

        *pixels_red   = (float*)calloc(image_size, sizeof(float));
        *pixels_green = (float*)calloc(image_size, sizeof(float));
        *pixels_blue  = (float*)calloc(image_size, sizeof(float));

        demosaic_rgb(bayered_pixels, *pixels_red, *pixels_green, *pixels_blue, *width, *height, filters, method);

        free(bayered_pixels);

        return 0;
    }
}
