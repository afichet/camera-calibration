#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <levmar.h>
#include <io.h>
#include <color-converter.h>

typedef struct {
    size_t n_patches;
    const float *reference_patches;
    const float *measured_patches;
} fit_params;


void measure(
    float *pParameters, float *pMeasurements,
    int n_parameters, int n_measurements,
    void *pUserParam
    )
{
    (void)n_parameters;
    (void)n_measurements;

    const fit_params* info = (fit_params*)pUserParam;

    float tristim_mea_corrected[3];
    float lab_ref[3];
    float lab_mea[3];

    for (size_t patch_idx = 0; patch_idx < info->n_patches; patch_idx++) {
        const float* tristim_ref = &(info->reference_patches[3*patch_idx]);
        const float* tristim_mea = &(info->measured_patches[3*patch_idx]);

        tristim_mea_corrected[0] = pParameters[0]*tristim_mea[0] + pParameters[1]*tristim_mea[1] + pParameters[2]*tristim_mea[2];
        tristim_mea_corrected[1] = pParameters[3]*tristim_mea[0] + pParameters[4]*tristim_mea[1] + pParameters[5]*tristim_mea[2];
        tristim_mea_corrected[2] = pParameters[6]*tristim_mea[0] + pParameters[7]*tristim_mea[1] + pParameters[8]*tristim_mea[2];
        
        XYZ_to_Lab(tristim_ref, lab_ref);
        XYZ_to_Lab(tristim_mea_corrected, lab_mea);    
        
        pMeasurements[patch_idx] = deltaE_2000(lab_ref, lab_mea);
    }
}

int main(int argc, const char* argv[]) 
{
    if (argc < 4) {
        printf(
            "Usage:\n"
            "------\n"
            "extract-matrix <data_xyz_ref> <data_measured> <output_matrix>\n"
            );

        return 0;
    }

    float* macbeth_patches_reference_xyz = NULL;
    float* macbeth_patches_captured = NULL;
    size_t size = 0;

    load_xyz(argv[1], &macbeth_patches_reference_xyz, &size);
    load_xyz(argv[2], &macbeth_patches_captured, &size);

    float matrix[9] = {
        1.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        0.f, 0.f, 1.f
    };

    fit_params u_params = {
        size, macbeth_patches_reference_xyz, macbeth_patches_captured
    };

    slevmar_dif(measure, matrix, NULL, 9, size, 1000, NULL, NULL, NULL, NULL, &u_params);    
    save_xyz(argv[3], matrix, 3);

    free(macbeth_patches_reference_xyz);
    free(macbeth_patches_captured);

    return 0;
}