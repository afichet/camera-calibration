#include <iostream>
#include <array>
#include <cstdio>

#define cimg_display 0
#include "CImg.h"

void parse_matrix(const char* mat_file, std::array<float, 9>& matrix) {
  FILE* fin = std::fopen(mat_file, "r");

  fscanf(fin,
	 "%f, %f, %f,\n%f, %f, %f,\n%f, %f, %f",
	 &matrix[0], &matrix[1], &matrix[2],
	 &matrix[3], &matrix[4], &matrix[5],
	 &matrix[6], &matrix[7], &matrix[8]);
  
  std::fclose(fin);
}

inline void mat_mul(const std::array<float, 9>& matrix,
	     const std::array<float, 3>& col_in,
	     std::array<float, 3>& col_out) {
  col_out[0] = matrix[0]*col_in[0] + matrix[1]*col_in[1] + matrix[2]*col_in[2];
  col_out[1] = matrix[3]*col_in[0] + matrix[4]*col_in[1] + matrix[5]*col_in[2];
  col_out[2] = matrix[6]*col_in[0] + matrix[7]*col_in[1] + matrix[8]*col_in[2]; 
}

inline float from_sRGB(float c) {
  if (c <= 0.04045F) { return c / 12.92F; }
  return std::pow((c + 0.055F) / 1.055, 2.4F);
}

inline float to_sRGB(float c) {
  if (c <= 0.F) { return 0.F; }
  if (c >= 1.F) { return 1.F; }
  if (c <= 0.0031308F) { return 12.92F * c; }
  return 1.055 * std::pow(c, 0.41666) - 0.055;
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    std::cout << "Usage:" << std::endl
	      << "------" << std::endl
	      << argv[0] << " <image in> <matrix file> <image out>" << std::endl;
    return 0;
  }

  const char* f_img_in  = argv[1];
  const char* f_mat_in  = argv[2];
  const char* f_img_out = argv[3];

  std::array<float, 9> matrix;
  parse_matrix(f_mat_in, matrix);

  std::array<float, 9> xyz_to_rgb =
    { 3.2404542,-1.5371385,-0.4985314,
     -0.9692660, 1.8760108, 0.0415560,
     0.0556434,-0.2040259, 1.0572252};
  
  cimg_library::CImg<unsigned short> img(f_img_in);

  #pragma omp parallel for schedule(static)
  for (int y = 0; y < img.height(); y++) {
    std::array<float, 3> buff;
    std::array<float, 3> corrected;
    for (int x = 0; x < img.width(); x++) {
      // We want linear RGB values
      for (int i = 0; i < 3; i++) {
	buff[i] = from_sRGB(float(img(x, y, i)) / 255.F);
      }

      // The computed matrix convert input colors to XYZ
      mat_mul(matrix, buff, corrected);

      // We now convert to RGB_D65
      mat_mul(xyz_to_rgb, corrected, buff);

      // And apply gamma correction
      for (int i = 0; i < 3; i++) {
	img(x, y, i) = (unsigned short)(to_sRGB(buff[i]) * 255.F);
      }
    }
  }

  img.save(f_img_out);
}
