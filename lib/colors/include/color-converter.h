#ifndef COLOR_CONVERTER_H_
#define COLOR_CONVERTER_H_

void  XYZ_to_Lab(const float *XYZ, float *Lab);
void  XYZ_to_RGB(const float *XYZ, float *RGB);
float from_sRGB(float c);
float to_sRGB(float c);
float deltaE_2000(const float *Lab1, const float *Lab2);

#endif   // COLOR_CONVERTER_H_
