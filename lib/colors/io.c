#include <stdio.h>
#include <stdlib.h>


void read_matrix(const char* filename, float* matrix) 
{
    FILE* fin = fopen(filename, "r");

    fscanf(fin,
        "%f, %f, %f,\n%f, %f, %f,\n%f, %f, %f",
        &matrix[0], &matrix[1], &matrix[2],
        &matrix[3], &matrix[4], &matrix[5],
        &matrix[6], &matrix[7], &matrix[8]);

    fclose(fin);
}


void read_spd(
    const char* filename, 
    int** wavelengths, 
    float** values, 
    size_t* size) 
{
    FILE* fin = fopen(filename, "r");

    int* buff_wavelengths = NULL;
    float* buff_values = NULL;
    *size = 0;

    while (!feof(fin)) {
        (*size)++;
        buff_wavelengths = (int*)realloc(buff_wavelengths, (*size) * sizeof(int));    
        buff_values = (float*)realloc(buff_values, (*size) * sizeof(float));    

        fscanf(fin, "%d,%f\n", &buff_wavelengths[*size - 1], &buff_values[*size - 1]);
    }

    fclose(fin);

    *wavelengths = buff_wavelengths;
    *values = buff_values;
}


void read_cmfs(
    const char* filename, 
    int** wavelengths, 
    float** values_x, 
    float** values_y, 
    float** values_z, 
    size_t* size)
{
    FILE* fin = fopen(filename, "r");

    int* buff_wavelengths = NULL;
    float* buff_values_x = NULL;
    float* buff_values_y = NULL;
    float* buff_values_z = NULL;
    *size = 0;

    while (!feof(fin)) {
        (*size)++;
        buff_wavelengths = (int*)realloc(buff_wavelengths, (*size) * sizeof(int));    
        buff_values_x = (float*)realloc(buff_values_x, (*size) * sizeof(float));    
        buff_values_y = (float*)realloc(buff_values_y, (*size) * sizeof(float));    
        buff_values_z = (float*)realloc(buff_values_z, (*size) * sizeof(float));    

        fscanf(
            fin, "%d,%f,%f,%f\n", 
            &buff_wavelengths[*size - 1], 
            &buff_values_x[*size - 1],
            &buff_values_y[*size - 1],
            &buff_values_z[*size - 1]
            );
    }

    fclose(fin);

    *wavelengths = buff_wavelengths;
    *values_x = buff_values_x;
    *values_y = buff_values_y;
    *values_z = buff_values_z;
}


void load_xyz(
    const char* filename,
    float **xyz,
    size_t *size)
{
    FILE* fin = fopen(filename, "r");

    float* buff_values_xyz = NULL;
    *size = 0;

    while (!feof(fin)) {
        (*size)++;
        buff_values_xyz = (float*)realloc(buff_values_xyz, 3 * (*size) * sizeof(float));  

        fscanf(
            fin, "%f,%f,%f\n", 
            &buff_values_xyz[3*(*size - 1)],
            &buff_values_xyz[3*(*size - 1) + 1],
            &buff_values_xyz[3*(*size - 1) + 2]
            );
    }

    fclose(fin);

    *xyz = buff_values_xyz;
}


void save_xyz(
    const char* filename,
    const float *xyz,
    size_t size)
{
    FILE* fout = fopen(filename, "w");

    for (size_t i = 0; i < size; i++) {
        fprintf(
            fout, "%f,%f,%f\n",
            xyz[3*i], 
            xyz[3*i + 1],
            xyz[3*i + 2]);
    }

    fclose(fout);
}