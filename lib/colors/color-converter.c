#include <math.h>
#include <color-converter.h>

#ifndef M_PI
#    define M_PI 3.14159265358979323846 /* pi */
#endif

void matmul(const float* matrix, const float* color_in, float* color_out)
{
    color_out[0] = matrix[0] * color_in[0] + matrix[1] * color_in[1] + matrix[2] * color_in[2];
    color_out[1] = matrix[3] * color_in[0] + matrix[4] * color_in[1] + matrix[5] * color_in[2];
    color_out[2] = matrix[6] * color_in[0] + matrix[7] * color_in[1] + matrix[8] * color_in[2];
}


// http://www.brucelindbloom.com/index.html?Eqn_DeltaE_CIE2000.html

// TODO
const float refXYZ[3] = {.95047f, 1.f, 1.08883f};

void XYZ_to_Lab(const float* XYZ, float* Lab)
{
    float vXYZ[3];

    for (int i = 0; i < 3; i++) {
        vXYZ[i] = XYZ[i] / refXYZ[i];

        if (vXYZ[i] > 0.008856) {
            vXYZ[i] = powf(vXYZ[i], 1.f / 3.f);
        } else {
            vXYZ[i] = (903.3f * vXYZ[i] + 16.f) / 116.f;
        }
    }

    Lab[0] = (116.f * vXYZ[1]) - 16.f;
    Lab[1] = 500.f * (vXYZ[0] - vXYZ[1]);
    Lab[2] = 200.f * (vXYZ[1] - vXYZ[2]);
}


void XYZ_to_RGB(const float* XYZ, float* RGB)
{
    static const float m[9] = {
      3.2404542f,
      -1.5371385f,
      -0.4985314f,
      -0.9692660f,
      1.8760108f,
      0.0415560f,
      0.0556434f,
      -0.2040259f,
      1.0572252f};

    matmul(m, XYZ, RGB);
}


float from_sRGB(float c)
{
    if (c <= 0.04045f) {
        return c / 12.92f;
    }
    return powf((c + 0.055f) / 1.055f, 2.4F);
}


float to_sRGB(float c)
{
    if (c <= 0.f) {
        return 0.f;
    }
    if (c >= 1.f) {
        return 1.f;
    }
    if (c <= 0.0031308f) {
        return 12.92f * c;
    }
    return 1.055f * powf(c, 0.41666f) - 0.055f;
}


float deg2rad(float deg)
{
    return deg * (float)M_PI / 180.f;
}

float rad2deg(float rad)
{
    return rad * 180.f / (float)M_PI;
}

float deltaE_2000(const float* Lab1, const float* Lab2)
{
    const float barLprime = (Lab2[0] + Lab1[0]) / 2.f;

    const float C1        = sqrtf(Lab1[1] * Lab1[1] + Lab1[2] * Lab1[2]);
    const float C2        = sqrtf(Lab2[1] * Lab2[1] + Lab2[2] * Lab2[2]);
    const float Cbar      = (C1 + C2) / 2.f;
    const float CbarP7    = powf(Cbar, 7.f);
    const float p7        = powf(25.f, 7.f);
    const float G         = 0.5f * (1.f - sqrtf(CbarP7 / (CbarP7 + p7)));
    const float a1prime   = Lab1[1] * (1.f + G);
    const float a2prime   = Lab2[1] * (1.f + G);
    const float C1prime   = sqrtf(a1prime * a1prime + Lab1[2] * Lab1[2]);
    const float C2prime   = sqrtf(a2prime * a2prime + Lab2[2] * Lab2[2]);
    const float barCprime = (C1prime + C2prime) / 2.f;

    float h1prime = rad2deg(atan2f(Lab1[2], a1prime));
    if (h1prime < 0.f) {
        h1prime += 360.f;
    }

    float h2prime = rad2deg(atan2f(Lab2[2], a2prime));
    if (h2prime < 0.f) {
        h2prime += 360.f;
    }

    float barHprime;
    if (fabsf(h1prime - h2prime) > 180.f) {
        barHprime = (h1prime + h2prime + 360.f) / 2.f;
    } else {
        barHprime = (h1prime + h2prime) / 2.f;
    }

    const float T = 1.f - .17f * cosf(deg2rad(barHprime - 30.f)) + .24f * cosf(deg2rad(2.f * barHprime))
                    + .32f * cosf(deg2rad(3.f * barHprime + 6.f)) - .20f * cosf(deg2rad(4.f * barHprime - 63.f));

    float deltahprime;
    if (fabsf(h2prime - h1prime) <= 180.f) {
        deltahprime = h2prime - h1prime;
    } else if (h2prime <= h1prime) {
        deltahprime = h2prime - h1prime + 360.f;
    } else {
        deltahprime = h2prime - h1prime - 360.f;
    }
    const float dLprime = Lab2[0] - Lab1[0];
    const float dCprime = C2prime - C1prime;
    const float dHprime = 2.f * sqrtf(C1prime * C2prime) * sinf(deg2rad(deltahprime) / 2.f);

    float halfbarLprimeSqr = barLprime - 50.f;
    halfbarLprimeSqr *= halfbarLprimeSqr;

    const float SL = 1.f + (.015f * halfbarLprimeSqr) / sqrtf(20.f + halfbarLprimeSqr);
    const float SC = 1.f + 0.045f * barCprime;
    const float SH = 1.f + 0.015f * barCprime * T;

    const float expH   = (barHprime - 275.f) / 25.f;
    const float dTheta = 30.f * expf(-expH * expH);

    const float barCprimeP7 = powf(barCprime, 7.f);
    const float RC          = 2.f * sqrtf(barCprimeP7 / (barCprimeP7 + p7));
    const float RT          = -RC * sinf(2.f * deg2rad(dTheta));

    const float KL = 1.f;
    const float KC = 1.f;
    const float KH = 1.f;

    const float xdL = dLprime / (KL * SL);
    const float xdC = dCprime / (KC * SC);
    const float xdH = dHprime / (KH * SH);

    return sqrtf(xdL * xdL + xdC * xdC + xdH * xdH + RT * (dCprime / (KC * SC)) * (dHprime / (KH * SH)));
}
