
#include <imageprocessing.h>
#include <demosaicing.h>

#include <cstring>
#include <iostream>
#include <climits>

void amaze_demosaic_RT(const float *in, float *out, int width, int height, const unsigned int filters);

extern "C"
{
  void demosaic_rgb(
      const float *     bayered_image,
      float *           pixels_red,
      float *           pixels_green,
      float *           pixels_blue,
      size_t            width,
      size_t            height,
      unsigned int      filters,
      RAWDemosaicMethod method)
  {
    switch (method)
    {
      case NONE:
        no_demosaic_rgb(bayered_image, pixels_red, pixels_green, pixels_blue, width, height, filters);
        break;

      case BASIC:
        basic_demosaic_rgb(bayered_image, pixels_red, pixels_green, pixels_blue, width, height, filters);
        break;

      case VNG4:
        vng4_demosaic_rgb(bayered_image, pixels_red, pixels_green, pixels_blue, width, height, filters);
        break;

      case AHD:
        ahd_demosaic_rgb(bayered_image, pixels_red, pixels_green, pixels_blue, width, height, filters);
        break;

      case RCD:
        rcd_demosaic_rgb(bayered_image, pixels_red, pixels_green, pixels_blue, width, height, filters);
        break;

      case AMAZE:
        amaze_demosaic_rgb(bayered_image, pixels_red, pixels_green, pixels_blue, width, height, filters);
        break;
    }
  }

  void demosaic(
      const float *     bayered_image,
      float *           debayered_image,
      size_t            width,
      size_t            height,
      unsigned int      filters,
      RAWDemosaicMethod method)
  {
    switch (method)
    {
      case NONE:
        no_demosaic(bayered_image, debayered_image, width, height, filters);
        break;

      case BASIC:
        basic_demosaic(bayered_image, debayered_image, width, height, filters);
        break;

      case VNG4:
        vgn4_demosaic(bayered_image, debayered_image, width, height, filters);
        break;

      case AHD:
        ahd_demosaic(bayered_image, debayered_image, width, height, filters);
        break;

      case RCD:
        rcd_demosaic(bayered_image, debayered_image, width, height, filters);
        break;

      case AMAZE:
        amaze_demosaic(bayered_image, debayered_image, width, height, filters);
        break;
    }
  }


  void no_demosaic_rgb(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters)
  {
    float *bayer[4];

    memset(pixels_red, 0, width * height * sizeof(float));
    memset(pixels_green, 0, width * height * sizeof(float));
    memset(pixels_blue, 0, width * height * sizeof(float));

    switch (filters)
    {
      case 0x16161616:   // BGGR
        bayer[0] = pixels_blue;
        bayer[1] = pixels_green;
        bayer[2] = pixels_green;
        bayer[3] = pixels_red;
        break;
      case 0x61616161:   // GRBG
        bayer[0] = pixels_green;
        bayer[1] = pixels_red;
        bayer[2] = pixels_blue;
        bayer[3] = pixels_green;
        break;
      case 0x49494949:   // GBRG
        bayer[0] = pixels_green;
        bayer[1] = pixels_blue;
        bayer[2] = pixels_red;
        bayer[3] = pixels_green;
        break;
      case 0x94949494:   // RGGB
        bayer[0] = pixels_red;
        bayer[1] = pixels_green;
        bayer[2] = pixels_green;
        bayer[3] = pixels_blue;
        break;
      default:
        return;
    }

    // Populate each color from the bayered image
    #pragma omp parallel for
    for (int y = 0; y < int(height); y += 2)
    {
      for (size_t x = 0; x < width; x += 2)
      {
        // clang-format off
        bayer[0][(y    ) * width + x    ] = bayered_image[(y    ) * width + x    ];
        bayer[1][(y    ) * width + x + 1] = bayered_image[(y    ) * width + x + 1];
        bayer[2][(y + 1) * width + x    ] = bayered_image[(y + 1) * width + x    ];
        bayer[3][(y + 1) * width + x + 1] = bayered_image[(y + 1) * width + x + 1];
        // clang-format on
      }
    }
  }


  void
  no_demosaic(const float *bayered_image, float *debayered_image, size_t width, size_t height, unsigned int filters)
  {
    float *debayered_r_temp = new float[width * height];
    float *debayered_g_temp = new float[width * height];
    float *debayered_b_temp = new float[width * height];

    no_demosaic_rgb(bayered_image, debayered_r_temp, debayered_g_temp, debayered_b_temp, width, height, filters);

    for (size_t row = 0; row < height; row++)
    {
      for (size_t col = 0; col < width; col++)
      {
        debayered_image[3 * (row * width + col) + 0] = debayered_r_temp[row * width + col];
        debayered_image[3 * (row * width + col) + 1] = debayered_g_temp[row * width + col];
        debayered_image[3 * (row * width + col) + 2] = debayered_b_temp[row * width + col];
      }
    }

    delete[] debayered_r_temp;
    delete[] debayered_g_temp;
    delete[] debayered_b_temp;
  }

  /**
   * Demosaicing
   * ===========
   *
   * We place the Bayer samples in the target images
   * For instance, if the Bayer pattern is:
   *
   * ┌───┬───┐
   * │ G │ B │
   * ├───┼───┤
   * │ R │ G │
   * └───┴───┘
   *
   *        with the corresponding bayered image...
   *
   *                         ┌────────────────┬────────────────┐
   *                         │ (x, y)         │ (x + 1, y)     │
   *                         ├────────────────┼────────────────┤
   *                         │ (x, y + 1)     │ (x + 1, y + 1) │
   *                         └────────────────┴────────────────┘
   *
   * Each buffer is populated as follows:
   *
   *       ┌────────────────┬──────────────────┐
   *       │ 0              │ 0                │
   *   R = ├────────────────┼──────────────────┤
   *       │ px(x, y + 1)   │ 0                │
   *       └────────────────┴──────────────────┘
   *
   *       ┌────────────────┬──────────────────┐
   *       │ px(x, y)       │ 0                │
   *   G = ├────────────────┼──────────────────┤
   *       │ 0              │ px(x + 1, y + 1) │
   *       └────────────────┴──────────────────┘
   *
   *       ┌────────────────┬──────────────────┐
   *       │ 0              │ px(x + 1, y)     │
   *   B = ├────────────────┼──────────────────┤
   *       │ 0              │ 0                │
   *       └────────────────┴──────────────────┘
   *
   * Notice, G receives two values.
   *
   * Then, we perform a convolution on each channel to fill in the missing
   * values.
   *
   * The final image (width, height, 3) is reconstructed by taking for each
   * pixel the R, G, B arrays (each, (width, height)).
   */
  void basic_demosaic_rgb(
      const float *bayered_image,
      float *      pixels_red,
      float *      pixels_green,
      float *      pixels_blue,
      size_t       width,
      size_t       height,
      unsigned int filters)
  {
    size_t n_elems = width * height;

    float *r_buffer = new float[n_elems];
    float *g_buffer = new float[n_elems];
    float *b_buffer = new float[n_elems];

    // Just create larger version of the original image with holes
    // at positions where there is no photosites
    no_demosaic_rgb(bayered_image, r_buffer, g_buffer, b_buffer, width, height, filters);

    // Now, do the convolutions
    // clang-format off
    float r_matrix[9] = {
      0.25f, 0.50f, 0.25f,
      0.50f, 1.00f, 0.50f,
      0.25f, 0.50f, 0.25f
    };

    float b_matrix[9] = {
      0.25f, 0.50f, 0.25f,
      0.50f, 1.00f, 0.50f,
      0.25f, 0.50f, 0.25f
    };

    float g_matrix[9] = {
      0.00f, 0.25f, 0.00f,
      0.25f, 1.00f, 0.25f,
      0.00f, 0.25f, 0.00f
    };
    // clang-format on

    image_convolve3x3(r_matrix, r_buffer, pixels_red, width, height);
    image_convolve3x3(g_matrix, g_buffer, pixels_green, width, height);
    image_convolve3x3(b_matrix, b_buffer, pixels_blue, width, height);

    delete[] r_buffer;
    delete[] g_buffer;
    delete[] b_buffer;
  }


  void
  basic_demosaic(const float *bayered_image, float *debayered_image, size_t width, size_t height, unsigned int filters)
  {
    float *debayered_r_temp = new float[width * height];
    float *debayered_g_temp = new float[width * height];
    float *debayered_b_temp = new float[width * height];

    basic_demosaic_rgb(bayered_image, debayered_r_temp, debayered_g_temp, debayered_b_temp, width, height, filters);

    for (size_t row = 0; row < height; row++)
    {
      for (size_t col = 0; col < width; col++)
      {
        debayered_image[3 * (row * width + col) + 0] = debayered_r_temp[row * width + col];
        debayered_image[3 * (row * width + col) + 1] = debayered_g_temp[row * width + col];
        debayered_image[3 * (row * width + col) + 2] = debayered_b_temp[row * width + col];
      }
    }

    delete[] debayered_r_temp;
    delete[] debayered_g_temp;
    delete[] debayered_b_temp;
  }


  void amaze_demosaic_rgb(
      const float *      bayered_image,
      float *            pixels_red,
      float *            pixels_green,
      float *            pixels_blue,
      size_t             width,
      size_t             height,
      const unsigned int filters)
  {
    float *debayered_temp = new float[4 * width * height];

    amaze_demosaic_RT(bayered_image, debayered_temp, width, height, filters);

    for (size_t row = 0; row < height; row++)
    {
      for (size_t col = 0; col < width; col++)
      {
        pixels_red[row * width + col]   = debayered_temp[(row * width + col) * 4 + 0];
        pixels_green[row * width + col] = debayered_temp[(row * width + col) * 4 + 1];
        pixels_blue[row * width + col]  = debayered_temp[(row * width + col) * 4 + 2];
      }
    }

    delete[] debayered_temp;
  }


  void
  amaze_demosaic(const float *bayered_image, float *debayered_image, size_t width, size_t height, unsigned int filters)
  {
    float *debayered_temp = new float[4 * width * height];

    amaze_demosaic_RT(bayered_image, debayered_temp, width, height, filters);

    for (size_t row = 0; row < height; row++)
    {
      for (size_t col = 0; col < width; col++)
      {
        for (int c = 0; c < 3; c++)
          debayered_image[3 * (row * width + col) + c] = debayered_temp[4 * (row * width + col) + c];
      }
    }

    delete[] debayered_temp;
  }
}

/*
    This file is part of darktable,
    Copyright (C) 2011-2020 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#define __STDC_FORMAT_MACROS

#if defined(__SSE__)
#  include <xmmintrin.h>
#endif


#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstddef>


#ifndef MIN
#  define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#  define MAX(a, b) (((a) < (b)) ? (b) : (a))
#endif


/** Calculate the bayer pattern color from the row and column **/
static inline unsigned int FC(const size_t row, const size_t col, const uint32_t filters)
{
  return filters >> (((row << 1 & 14) + (col & 1)) << 1) & 3;
}


static __inline float clampnan(const float x, const float m, const float M)
{
  float r;
#warning seems wrong...

  // clamp to [m, M] if x is infinite; return average of m and M if x is NaN; else just return x

  if (std::isinf(x))
    r = (std::isless(x, m) ? m : (std::isgreater(x, M) ? M : x));
  else if (std::isnan(x))
    r = (m + M) / 2.0f;
  else   // normal number
    r = x;

  return r;
}

#ifndef __SSE2__
static __inline float xmul2f(float d)
{
  union
  {
    float    f;
    uint32_t u;
  } x;
  x.f = d;
  if (x.u & 0x7FFFFFFF)   // if f==0 do nothing
  {
    x.u += 1 << 23;   // add 1 to the exponent
  }
  return x.f;
}
#endif

static __inline float xdiv2f(float d)
{
  union
  {
    float    f;
    uint32_t u;
  } x;
  x.f = d;
  if (x.u & 0x7FFFFFFF)   // if f==0 do nothing
  {
    x.u -= 1 << 23;   // sub 1 from the exponent
  }
  return x.f;
}

static __inline float xdivf(float d, int n)
{
  union
  {
    float    f;
    uint32_t u;
  } x;
  x.f = d;
  if (x.u & 0x7FFFFFFF)   // if f==0 do nothing
  {
    x.u -= n << 23;   // add n to the exponent
  }
  return x.f;
}


/*==================================================================================
 * begin raw therapee code, hg checkout of march 03, 2016 branch master.
 *==================================================================================*/

#define pow_F(a, b) (xexpf(b * xlogf(a)))

#ifdef __GNUC__
#  define RESTRICT    __restrict__
#  define LIKELY(x)   __builtin_expect(!!(x), 1)
#  define UNLIKELY(x) __builtin_expect(!!(x), 0)
#  define ALIGNED64   __attribute__((aligned(64)))
#  define ALIGNED16   __attribute__((aligned(16)))
#else
#  define RESTRICT
#  define LIKELY(x)   (x)
#  define UNLIKELY(x) (x)
#  define ALIGNED64
#  define ALIGNED16
#endif


#ifdef __SSE2__

#  ifdef __GNUC__
#    define INLINE __inline
#  else
#    define INLINE inline
#  endif

#  ifdef __GNUC__
#    if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 9) || __GNUC__ > 4) && (!defined(WIN32) || defined(__x86_64__))
#      define LVF(x)      _mm_load_ps(&x)
#      define LVFU(x)     _mm_loadu_ps(&x)
#      define STVF(x, y)  _mm_store_ps(&x, y)
#      define STVFU(x, y) _mm_storeu_ps(&x, y)
#    else   // there is a bug in gcc 4.7.x when using openmp and aligned memory and -O3, also need to map the
            // aligned functions to unaligned functions for WIN32 builds
#      define LVF(x)      _mm_loadu_ps(&x)
#      define LVFU(x)     _mm_loadu_ps(&x)
#      define STVF(x, y)  _mm_storeu_ps(&x, y)
#      define STVFU(x, y) _mm_storeu_ps(&x, y)
#    endif
#  else
#    define LVF(x)      _mm_load_ps(&x)
#    define LVFU(x)     _mm_loadu_ps(&x)
#    define STVF(x, y)  _mm_store_ps(&x, y)
#    define STVFU(x, y) _mm_storeu_ps(&x, y)
#  endif

#  define STC2VFU(a, v)                                                                                                \
    {                                                                                                                  \
      __m128 TST1V = _mm_loadu_ps(&a);                                                                                 \
      __m128 TST2V = _mm_unpacklo_ps(v, v);                                                                            \
      vmask  cmask = _mm_set_epi32(0xffffffff, 0, 0xffffffff, 0);                                                      \
      _mm_storeu_ps(&a, vself(cmask, TST1V, TST2V));                                                                   \
      TST1V = _mm_loadu_ps((&a) + 4);                                                                                  \
      TST2V = _mm_unpackhi_ps(v, v);                                                                                   \
      _mm_storeu_ps((&a) + 4, vself(cmask, TST1V, TST2V));                                                             \
    }

#  define ZEROV  _mm_setzero_ps()
#  define F2V(a) _mm_set1_ps((a))

typedef __m128i vmask;
typedef __m128  vfloat;
typedef __m128i vint;

static INLINE vfloat LC2VFU(float &a)
{
  // Load 8 floats from a and combine a[0],a[2],a[4] and a[6] into a vector of 4 floats
  vfloat a1 = _mm_loadu_ps(&a);
  vfloat a2 = _mm_loadu_ps((&a) + 4);
  return _mm_shuffle_ps(a1, a2, _MM_SHUFFLE(2, 0, 2, 0));
}
static INLINE vfloat vmaxf(vfloat x, vfloat y)
{
  return _mm_max_ps(x, y);
}
static INLINE vfloat vminf(vfloat x, vfloat y)
{
  return _mm_min_ps(x, y);
}
static INLINE vfloat vcast_vf_f(float f)
{
  return _mm_set_ps(f, f, f, f);
}
static INLINE vmask vorm(vmask x, vmask y)
{
  return _mm_or_si128(x, y);
}
static INLINE vmask vandm(vmask x, vmask y)
{
  return _mm_and_si128(x, y);
}
static INLINE vmask vandnotm(vmask x, vmask y)
{
  return _mm_andnot_si128(x, y);
}
static INLINE vfloat vabsf(vfloat f)
{
  return (vfloat)vandnotm((vmask)vcast_vf_f(-0.0f), (vmask)f);
}
static INLINE vfloat vself(vmask mask, vfloat x, vfloat y)
{
  return (vfloat)vorm(vandm(mask, (vmask)x), vandnotm(mask, (vmask)y));
}
static INLINE vmask vmaskf_lt(vfloat x, vfloat y)
{
  return (__m128i)_mm_cmplt_ps(x, y);
}
static INLINE vmask vmaskf_gt(vfloat x, vfloat y)
{
  return (__m128i)_mm_cmpgt_ps(x, y);
}
static INLINE vfloat ULIMV(vfloat a, vfloat b, vfloat c)
{
  // made to clamp a in range [b,c] but in fact it's also the median of a,b,c, which means that the result is
  // independent on order of arguments
  // ULIMV(a,b,c) = ULIMV(a,c,b) = ULIMV(b,a,c) = ULIMV(b,c,a) = ULIMV(c,a,b) = ULIMV(c,b,a)
  return vmaxf(vminf(a, b), vminf(vmaxf(a, b), c));
}
static INLINE vfloat SQRV(vfloat a)
{
  return a * a;
}
static INLINE vfloat vintpf(vfloat a, vfloat b, vfloat c)
{
  // calculate a * b + (1 - a) * c (interpolate two values)
  // following is valid:
  // vintpf(a, b+x, c+x) = vintpf(a, b, c) + x
  // vintpf(a, b*x, c*x) = vintpf(a, b, c) * x
  return a * (b - c) + c;
}
static INLINE vfloat vaddc2vfu(float &a)
{
  // loads a[0]..a[7] and returns { a[0]+a[1], a[2]+a[3], a[4]+a[5], a[6]+a[7] }
  vfloat a1 = _mm_loadu_ps(&a);
  vfloat a2 = _mm_loadu_ps((&a) + 4);
  return _mm_shuffle_ps(a1, a2, _MM_SHUFFLE(2, 0, 2, 0)) + _mm_shuffle_ps(a1, a2, _MM_SHUFFLE(3, 1, 3, 1));
}
static INLINE vfloat vadivapb(vfloat a, vfloat b)
{
  return a / (a + b);
}
static INLINE vint vselc(vmask mask, vint x, vint y)
{
  return vorm(vandm(mask, (vmask)x), vandnotm(mask, (vmask)y));
}
static INLINE vint vselinotzero(vmask mask, vint x)
{
  // returns value of x if corresponding mask bits are 0, else returns 0
  // faster than vselc(mask, ZEROV, x)
  return _mm_andnot_si128(mask, x);
}
static INLINE vfloat vmul2f(vfloat a)
{
  // fastest way to multiply by 2
  return a + a;
}
static INLINE vmask vmaskf_ge(vfloat x, vfloat y)
{
  return (__m128i)_mm_cmpge_ps(x, y);
}
static INLINE vmask vnotm(vmask x)
{
  return _mm_xor_si128(x, _mm_cmpeq_epi32(_mm_setzero_si128(), _mm_setzero_si128()));
}
static INLINE vfloat vdup(vfloat a)
{
  // returns { a[0],a[0],a[1],a[1] }
  return _mm_unpacklo_ps(a, a);
}

#endif   // __SSE2__

template<typename _Tp>
static inline const _Tp SQR(_Tp x)
{
  //      return std::pow(x,2); Slower than:
  return (x * x);
}

template<typename _Tp>
static inline const _Tp intp(const _Tp a, const _Tp b, const _Tp c)
{
  // calculate a * b + (1 - a) * c
  // following is valid:
  // intp(a, b+x, c+x) = intp(a, b, c) + x
  // intp(a, b*x, c*x) = intp(a, b, c) * x
  return a * (b - c) + c;
}

template<typename _Tp>
static inline const _Tp LIM(const _Tp a, const _Tp b, const _Tp c)
{
  return std::max(b, std::min(a, c));
}

template<typename _Tp>
static inline const _Tp ULIM(const _Tp a, const _Tp b, const _Tp c)
{
  return ((b < c) ? LIM(a, b, c) : LIM(a, c, b));
}

// Store a vector of 4 floats in a[0],a[2],a[4] and a[6]
#ifdef __SSE4_1__
// SSE4.1 => use _mm_blend_ps instead of _mm_set_epi32 and vself
#  define STC2VFU(a, v)                                                                                                \
    {                                                                                                                  \
      __m128 TST1V = _mm_loadu_ps(&a);                                                                                 \
      __m128 TST2V = _mm_unpacklo_ps(v, v);                                                                            \
      _mm_storeu_ps(&a, _mm_blend_ps(TST1V, TST2V, 5));                                                                \
      TST1V = _mm_loadu_ps((&a) + 4);                                                                                  \
      TST2V = _mm_unpackhi_ps(v, v);                                                                                   \
      _mm_storeu_ps((&a) + 4, _mm_blend_ps(TST1V, TST2V, 5));                                                          \
    }
#else
#  define STC2VFU(a, v)                                                                                                \
    {                                                                                                                  \
      __m128 TST1V = _mm_loadu_ps(&a);                                                                                 \
      __m128 TST2V = _mm_unpacklo_ps(v, v);                                                                            \
      vmask  cmask = _mm_set_epi32(0xffffffff, 0, 0xffffffff, 0);                                                      \
      _mm_storeu_ps(&a, vself(cmask, TST1V, TST2V));                                                                   \
      TST1V = _mm_loadu_ps((&a) + 4);                                                                                  \
      TST2V = _mm_unpackhi_ps(v, v);                                                                                   \
      _mm_storeu_ps((&a) + 4, vself(cmask, TST1V, TST2V));                                                             \
    }
#endif


////////////////////////////////////////////////////////////////
//
//          AMaZE demosaic algorithm
// (Aliasing Minimization and Zipper Elimination)
//
//  copyright (c) 2008-2010  Emil Martinec <ejmartin@uchicago.edu>
//  optimized for speed by Ingo Weyrich
//
// incorporating ideas of Luis Sanz Rodrigues and Paul Lee
//
// code dated: May 27, 2010
// latest modification: Ingo Weyrich, January 25, 2016
//
//  amaze_interpolate_RT.cc is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////

void amaze_demosaic_RT(const float *in, float *out, int width, int height, const unsigned int filters)
{
  int         winx    = 0;
  int         winy    = 0;
  const float clip_pt = 1.0;   //fminf(piece->pipe->dsc.processed_maximum[0],
  //  fminf(piece->pipe->dsc.processed_maximum[1], piece->pipe->dsc.processed_maximum[2]));
  const float clip_pt8 = 0.8f * clip_pt;

// this allows to pass AMAZETS to the code. On some machines larger AMAZETS is faster
// If AMAZETS is undefined it will be set to 160, which is the fastest on modern x86/64 machines
#ifndef AMAZETS
#  define AMAZETS 160
#endif
  // Tile size; the image is processed in square tiles to lower memory requirements and facilitate
  // multi-threading
  // We assure that Tile size is a multiple of 32 in the range [96;992]
  constexpr int ts  = (AMAZETS & 992) < 96 ? 96 : (AMAZETS & 992);
  constexpr int tsh = ts / 2;   // half of Tile size

  // offset of R pixel within a Bayer quartet
  int ex, ey;

  // determine GRBG coset; (ey,ex) is the offset of the R subarray
  if (FC(0, 0, filters) == 1)
  {   // first pixel is G
    if (FC(0, 1, filters) == 0)
    {
      ey = 0;
      ex = 1;
    }
    else
    {
      ey = 1;
      ex = 0;
    }
  }
  else
  {   // first pixel is R or B
    if (FC(0, 0, filters) == 0)
    {
      ey = 0;
      ex = 0;
    }
    else
    {
      ey = 1;
      ex = 1;
    }
  }

  // shifts of pointer value to access pixels in vertical and diagonal directions
  constexpr int v1 = ts, v2 = 2 * ts, v3 = 3 * ts, p1 = -ts + 1, p2 = -2 * ts + 2, p3 = -3 * ts + 3, m1 = ts + 1,
                m2 = 2 * ts + 2, m3 = 3 * ts + 3;

  // tolerance to avoid dividing by zero
  constexpr float eps = 1e-5, epssq = 1e-10;   // tolerance to avoid dividing by zero

  // adaptive ratios threshold
  constexpr float arthresh = 0.75;

  // gaussian on 5x5 quincunx, sigma=1.2
  constexpr float gaussodd[4] = {0.14659727707323927f, 0.103592713382435f, 0.0732036125103057f, 0.0365543548389495f};
  // nyquist texture test threshold
  constexpr float nyqthresh = 0.5;
  // gaussian on 5x5, sigma=1.2, multiplied with nyqthresh to save some time later in loop
  // Is this really sigma=1.2????, seems more like sigma = 1.672
  constexpr float gaussgrad[6]
      = {nyqthresh * 0.07384411893421103f,
         nyqthresh * 0.06207511968171489f,
         nyqthresh * 0.0521818194747806f,
         nyqthresh * 0.03687419286733595f,
         nyqthresh * 0.03099732204057846f,
         nyqthresh * 0.018413194161458882f};
  // gaussian on 5x5 alt quincunx, sigma=1.5
  constexpr float gausseven[2] = {0.13719494435797422f, 0.05640252782101291f};
  // gaussian on quincunx grid
  constexpr float gquinc[4] = {0.169917f, 0.108947f, 0.069855f, 0.0287182f};

  typedef struct
  {
    float h;
    float v;
  } s_hv;

#ifdef _OPENMP
//#pragma omp parallel
#endif
  {
    //     int progresscounter = 0;

    constexpr int cldf = 2;   // factor to multiply cache line distance. 1 = 64 bytes, 2 = 128 bytes ...
    // assign working space
    char *buffer = (char *)calloc(14 * sizeof(float) * ts * ts + sizeof(char) * ts * tsh + 18 * cldf * 64 + 63, 1);
    // aligned to 64 byte boundary
    char *data = (char *)((uintptr_t(buffer) + uintptr_t(63)) / 64 * 64);

    // green values
    float *rgbgreen = (float(*))data;
    // sum of square of horizontal gradient and square of vertical gradient
    float *delhvsqsum = (float(*))((char *)rgbgreen + sizeof(float) * ts * ts + cldf * 64);   // 1
    // gradient based directional weights for interpolation
    float *dirwts0 = (float(*))((char *)delhvsqsum + sizeof(float) * ts * ts + cldf * 64);   // 1
    float *dirwts1 = (float(*))((char *)dirwts0 + sizeof(float) * ts * ts + cldf * 64);      // 1
    // vertically interpolated colour differences G-R, G-B
    float *vcd = (float(*))((char *)dirwts1 + sizeof(float) * ts * ts + cldf * 64);   // 1
    // horizontally interpolated colour differences
    float *hcd = (float(*))((char *)vcd + sizeof(float) * ts * ts + cldf * 64);   // 1
    // alternative vertical interpolation
    float *vcdalt = (float(*))((char *)hcd + sizeof(float) * ts * ts + cldf * 64);   // 1
    // alternative horizontal interpolation
    float *hcdalt = (float(*))((char *)vcdalt + sizeof(float) * ts * ts + cldf * 64);   // 1
    // square of average colour difference
    float *cddiffsq = (float(*))((char *)hcdalt + sizeof(float) * ts * ts + cldf * 64);   // 1
    // weight to give horizontal vs vertical interpolation
    float *hvwt = (float(*))((char *)cddiffsq + sizeof(float) * ts * ts + 2 * cldf * 64);   // 1
    // final interpolated colour difference
    float(*Dgrb)[ts * tsh] = (float(*)[ts * tsh]) vcdalt;   // there is no overlap in buffer usage => share
    // gradient in plus (NE/SW) direction
    float *delp = (float(*))cddiffsq;   // there is no overlap in buffer usage => share
    // gradient in minus (NW/SE) direction
    float *delm = (float(*))((char *)delp + sizeof(float) * ts * tsh + cldf * 64);
    // diagonal interpolation of R+B
    float *rbint = (float(*))delm;   // there is no overlap in buffer usage => share
    // horizontal and vertical curvature of interpolated G (used to refine interpolation in Nyquist texture
    // regions)
    s_hv *Dgrb2 = (s_hv(*))((char *)hvwt + sizeof(float) * ts * tsh + cldf * 64);   // 1
    // difference between up/down interpolations of G
    float *dgintv = (float(*))Dgrb2;   // there is no overlap in buffer usage => share
    // difference between left/right interpolations of G
    float *dginth = (float(*))((char *)dgintv + sizeof(float) * ts * ts + cldf * 64);   // 1
    // square of diagonal colour differences
    float *Dgrbsq1m = (float(*))((char *)dginth + sizeof(float) * ts * ts + cldf * 64);      // 1
    float *Dgrbsq1p = (float(*))((char *)Dgrbsq1m + sizeof(float) * ts * tsh + cldf * 64);   // 1
    // tile raw data
    float *cfa = (float(*))((char *)Dgrbsq1p + sizeof(float) * ts * tsh + cldf * 64);   // 1
    // relative weight for combining plus and minus diagonal interpolations
    float *pmwt = (float(*))delhvsqsum;   // there is no overlap in buffer usage => share
    // interpolated colour difference R-B in minus and plus direction
    float *rbm = (float(*))vcd;   // there is no overlap in buffer usage => share
    float *rbp = (float(*))((char *)rbm + sizeof(float) * ts * tsh + cldf * 64);
    // nyquist texture flags 1=nyquist, 0=not nyquist
    unsigned char *nyquist  = (unsigned char(*))((char *)cfa + sizeof(float) * ts * ts + cldf * 64);   // 1
    unsigned char *nyquist2 = (unsigned char(*))cddiffsq;
    float *        nyqutest = (float(*))((char *)nyquist + sizeof(unsigned char) * ts * tsh + cldf * 64);   // 1

// Main algorithm: Tile loop
// use collapse(2) to collapse the 2 loops to one large loop, so there is better scaling
#ifdef _OPENMP
////#pragma omp for SIMD() schedule(static) collapse(2) nowait
#endif

    for (int top = winy - 16; top < winy + height; top += ts - 32)
    {
      for (int left = winx - 16; left < winx + width; left += ts - 32)
      {
        memset(&nyquist[3 * tsh], 0, sizeof(unsigned char) * (ts - 6) * tsh);
        // location of tile bottom edge
        int bottom = MIN(top + ts, winy + height + 16);
        // location of tile right edge
        int right = MIN(left + ts, winx + width + 16);
        // tile width  (=ts except for right edge of image)
        int rr1 = bottom - top;
        // tile height (=ts except for bottom edge of image)
        int cc1 = right - left;
        // bookkeeping for borders
        // min and max row/column in the tile
        int rrmin = top < winy ? 16 : 0;
        int ccmin = left < winx ? 16 : 0;
        int rrmax = bottom > (winy + height) ? winy + height - top : rr1;
        int ccmax = right > (winx + width) ? winx + width - left : cc1;

// rgb from input CFA data
// rgb values should be floating point number between 0 and 1
// after white balance multipliers are applied
// a 16 pixel border is added to each side of the image

// begin of tile initialization
#ifdef __SSE2__
        // fill upper border
        if (rrmin > 0)
        {
          for (int rr = 0; rr < 16; rr++)
          {
            int row = 32 - rr + top;

            for (int cc = ccmin; cc < ccmax; cc += 4)
            {
              int    indx1 = rr * ts + cc;
              vfloat tempv = LVFU(in[row * width + (cc + left)]);
              STVF(cfa[indx1], tempv);
              STVF(rgbgreen[indx1], tempv);
            }
          }
        }

        // fill inner part
        for (int rr = rrmin; rr < rrmax; rr++)
        {
          int row = rr + top;

          for (int cc = ccmin; cc < ccmax; cc += 4)
          {
            int    indx1 = rr * ts + cc;
            vfloat tempv = LVFU(in[row * width + (cc + left)]);
            STVF(cfa[indx1], tempv);
            STVF(rgbgreen[indx1], tempv);
          }
        }

        // fill lower border
        if (rrmax < rr1)
        {
          for (int rr = 0; rr < 16; rr++)
            for (int cc = ccmin; cc < ccmax; cc += 4)
            {
              int    indx1 = (rrmax + rr) * ts + cc;
              vfloat tempv = LVFU(in[(winy + height - rr - 2) * width + (left + cc)]);
              STVF(cfa[indx1], tempv);
              STVF(rgbgreen[indx1], tempv);
            }
        }

#else

        // fill upper border
        if (rrmin > 0)
        {
          for (int rr = 0; rr < 16; rr++)
            for (int cc = ccmin, row = 32 - rr + top; cc < ccmax; cc++)
            {
              cfa[rr * ts + cc]      = (in[row * width + (cc + left)]);
              rgbgreen[rr * ts + cc] = cfa[rr * ts + cc];
            }
        }

        // fill inner part
        for (int rr = rrmin; rr < rrmax; rr++)
        {
          int row = rr + top;

          for (int cc = ccmin; cc < ccmax; cc++)
          {
            int indx1       = rr * ts + cc;
            cfa[indx1]      = (in[row * width + (cc + left)]);
            rgbgreen[indx1] = cfa[indx1];
          }
        }

        // fill lower border
        if (rrmax < rr1)
        {
          for (int rr = 0; rr < 16; rr++)
            for (int cc = ccmin; cc < ccmax; cc++)
            {
              cfa[(rrmax + rr) * ts + cc]      = (in[(winy + height - rr - 2) * width + (left + cc)]);
              rgbgreen[(rrmax + rr) * ts + cc] = cfa[(rrmax + rr) * ts + cc];
            }
        }

#endif

        // fill left border
        if (ccmin > 0)
        {
          for (int rr = rrmin; rr < rrmax; rr++)
            for (int cc = 0, row = rr + top; cc < 16; cc++)
            {
              cfa[rr * ts + cc]      = (in[row * width + (32 - cc + left)]);
              rgbgreen[rr * ts + cc] = cfa[rr * ts + cc];
            }
        }

        // fill right border
        if (ccmax < cc1)
        {
          for (int rr = rrmin; rr < rrmax; rr++)
            for (int cc = 0; cc < 16; cc++)
            {
              cfa[rr * ts + ccmax + cc]      = (in[(top + rr) * width + ((winx + width - cc - 2))]);
              rgbgreen[rr * ts + ccmax + cc] = cfa[rr * ts + ccmax + cc];
            }
        }

        // also, fill the image corners
        if (rrmin > 0 && ccmin > 0)
        {
          for (int rr = 0; rr < 16; rr++)
            for (int cc = 0; cc < 16; cc++)
            {
              cfa[(rr)*ts + cc]      = (in[(winy + 32 - rr) * width + (winx + 32 - cc)]);
              rgbgreen[(rr)*ts + cc] = cfa[(rr)*ts + cc];
            }
        }

        if (rrmax < rr1 && ccmax < cc1)
        {
          for (int rr = 0; rr < 16; rr++)
            for (int cc = 0; cc < 16; cc++)
            {
              cfa[(rrmax + rr) * ts + ccmax + cc] = (in[(winy + height - rr - 2) * width + ((winx + width - cc - 2))]);
              rgbgreen[(rrmax + rr) * ts + ccmax + cc] = cfa[(rrmax + rr) * ts + ccmax + cc];
            }
        }

        if (rrmin > 0 && ccmax < cc1)
        {
          for (int rr = 0; rr < 16; rr++)
            for (int cc = 0; cc < 16; cc++)
            {
              cfa[(rr)*ts + ccmax + cc]      = (in[(winy + 32 - rr) * width + ((winx + width - cc - 2))]);
              rgbgreen[(rr)*ts + ccmax + cc] = cfa[(rr)*ts + ccmax + cc];
            }
        }

        if (rrmax < rr1 && ccmin > 0)
        {
          for (int rr = 0; rr < 16; rr++)
            for (int cc = 0; cc < 16; cc++)
            {
              cfa[(rrmax + rr) * ts + cc]      = (in[(winy + height - rr - 2) * width + ((winx + 32 - cc))]);
              rgbgreen[(rrmax + rr) * ts + cc] = cfa[(rrmax + rr) * ts + cc];
            }
        }

// end of tile initialization

// horizontal and vertical gradients
#ifdef __SSE2__
        vfloat epsv = F2V(eps);

        for (int rr = 2; rr < rr1 - 2; rr++)
        {
          for (int indx = rr * ts; indx < rr * ts + cc1; indx += 4)
          {
            vfloat delhv = vabsf(LVFU(cfa[indx + 1]) - LVFU(cfa[indx - 1]));
            vfloat delvv = vabsf(LVF(cfa[indx + v1]) - LVF(cfa[indx - v1]));
            STVF(
                dirwts1[indx],
                epsv + vabsf(LVFU(cfa[indx + 2]) - LVF(cfa[indx])) + vabsf(LVF(cfa[indx]) - LVFU(cfa[indx - 2]))
                    + delhv);
            STVF(
                dirwts0[indx],
                epsv + vabsf(LVF(cfa[indx + v2]) - LVF(cfa[indx])) + vabsf(LVF(cfa[indx]) - LVF(cfa[indx - v2]))
                    + delvv);
            STVF(delhvsqsum[indx], SQRV(delhv) + SQRV(delvv));
          }
        }

#else

        for (int rr = 2; rr < rr1 - 2; rr++)
          for (int cc = 2, indx = (rr)*ts + cc; cc < cc1 - 2; cc++, indx++)
          {
            float delh       = fabsf(cfa[indx + 1] - cfa[indx - 1]);
            float delv       = fabsf(cfa[indx + v1] - cfa[indx - v1]);
            dirwts0[indx]    = eps + fabsf(cfa[indx + v2] - cfa[indx]) + fabsf(cfa[indx] - cfa[indx - v2]) + delv;
            dirwts1[indx]    = eps + fabsf(cfa[indx + 2] - cfa[indx]) + fabsf(cfa[indx] - cfa[indx - 2]) + delh;
            delhvsqsum[indx] = SQR(delh) + SQR(delv);
          }

#endif

// interpolate vertical and horizontal colour differences
#ifdef __SSE2__
        vfloat sgnv;

        if (!(FC(4, 4, filters) & 1))
        {
          sgnv = _mm_set_ps(1.f, -1.f, 1.f, -1.f);
        }
        else
        {
          sgnv = _mm_set_ps(-1.f, 1.f, -1.f, 1.f);
        }

        vfloat zd5v      = F2V(0.5f);
        vfloat onev      = F2V(1.f);
        vfloat arthreshv = F2V(arthresh);
        vfloat clip_pt8v = F2V(clip_pt8);

        for (int rr = 4; rr < rr1 - 4; rr++)
        {
          sgnv = -sgnv;

          for (int indx = rr * ts + 4; indx < rr * ts + cc1 - 7; indx += 4)
          {
            // colour ratios in each cardinal direction
            vfloat cfav = LVF(cfa[indx]);
            vfloat cruv
                = LVF(cfa[indx - v1]) * (LVF(dirwts0[indx - v2]) + LVF(dirwts0[indx]))
                  / (LVF(dirwts0[indx - v2]) * (epsv + cfav) + LVF(dirwts0[indx]) * (epsv + LVF(cfa[indx - v2])));
            vfloat crdv
                = LVF(cfa[indx + v1]) * (LVF(dirwts0[indx + v2]) + LVF(dirwts0[indx]))
                  / (LVF(dirwts0[indx + v2]) * (epsv + cfav) + LVF(dirwts0[indx]) * (epsv + LVF(cfa[indx + v2])));
            vfloat crlv
                = LVFU(cfa[indx - 1]) * (LVFU(dirwts1[indx - 2]) + LVF(dirwts1[indx]))
                  / (LVFU(dirwts1[indx - 2]) * (epsv + cfav) + LVF(dirwts1[indx]) * (epsv + LVFU(cfa[indx - 2])));
            vfloat crrv
                = LVFU(cfa[indx + 1]) * (LVFU(dirwts1[indx + 2]) + LVF(dirwts1[indx]))
                  / (LVFU(dirwts1[indx + 2]) * (epsv + cfav) + LVF(dirwts1[indx]) * (epsv + LVFU(cfa[indx + 2])));

            // G interpolated in vert/hor directions using Hamilton-Adams method
            vfloat guhav = LVF(cfa[indx - v1]) + zd5v * (cfav - LVF(cfa[indx - v2]));
            vfloat gdhav = LVF(cfa[indx + v1]) + zd5v * (cfav - LVF(cfa[indx + v2]));
            vfloat glhav = LVFU(cfa[indx - 1]) + zd5v * (cfav - LVFU(cfa[indx - 2]));
            vfloat grhav = LVFU(cfa[indx + 1]) + zd5v * (cfav - LVFU(cfa[indx + 2]));

            // G interpolated in vert/hor directions using adaptive ratios
            vfloat guarv = vself(vmaskf_lt(vabsf(onev - cruv), arthreshv), cfav * cruv, guhav);
            vfloat gdarv = vself(vmaskf_lt(vabsf(onev - crdv), arthreshv), cfav * crdv, gdhav);
            vfloat glarv = vself(vmaskf_lt(vabsf(onev - crlv), arthreshv), cfav * crlv, glhav);
            vfloat grarv = vself(vmaskf_lt(vabsf(onev - crrv), arthreshv), cfav * crrv, grhav);

            // adaptive weights for vertical/horizontal directions
            vfloat hwtv = LVFU(dirwts1[indx - 1]) / (LVFU(dirwts1[indx - 1]) + LVFU(dirwts1[indx + 1]));
            vfloat vwtv = LVF(dirwts0[indx - v1]) / (LVF(dirwts0[indx + v1]) + LVF(dirwts0[indx - v1]));

            // interpolated G via adaptive weights of cardinal evaluations
            vfloat Ginthhav = vintpf(hwtv, grhav, glhav);
            vfloat Gintvhav = vintpf(vwtv, gdhav, guhav);

            // interpolated colour differences
            vfloat hcdaltv = sgnv * (Ginthhav - cfav);
            vfloat vcdaltv = sgnv * (Gintvhav - cfav);
            STVF(hcdalt[indx], hcdaltv);
            STVF(vcdalt[indx], vcdaltv);

            vmask clipmask = vorm(
                vorm(vmaskf_gt(cfav, clip_pt8v), vmaskf_gt(Gintvhav, clip_pt8v)),
                vmaskf_gt(Ginthhav, clip_pt8v));
            guarv = vself(clipmask, guhav, guarv);
            gdarv = vself(clipmask, gdhav, gdarv);
            glarv = vself(clipmask, glhav, glarv);
            grarv = vself(clipmask, grhav, grarv);

            // use HA if highlights are (nearly) clipped
            STVF(vcd[indx], vself(clipmask, vcdaltv, sgnv * (vintpf(vwtv, gdarv, guarv) - cfav)));
            STVF(hcd[indx], vself(clipmask, hcdaltv, sgnv * (vintpf(hwtv, grarv, glarv) - cfav)));

            // differences of interpolations in opposite directions
            STVF(dgintv[indx], vminf(SQRV(guhav - gdhav), SQRV(guarv - gdarv)));
            STVF(dginth[indx], vminf(SQRV(glhav - grhav), SQRV(glarv - grarv)));
          }
        }

#else

        for (int rr = 4; rr < rr1 - 4; rr++)
        {
          bool fcswitch = FC(rr, 4, filters) & 1;

          for (int cc = 4, indx = rr * ts + cc; cc < cc1 - 4; cc++, indx++)
          {
            // colour ratios in each cardinal direction
            float cru = cfa[indx - v1] * (dirwts0[indx - v2] + dirwts0[indx])
                        / (dirwts0[indx - v2] * (eps + cfa[indx]) + dirwts0[indx] * (eps + cfa[indx - v2]));
            float crd = cfa[indx + v1] * (dirwts0[indx + v2] + dirwts0[indx])
                        / (dirwts0[indx + v2] * (eps + cfa[indx]) + dirwts0[indx] * (eps + cfa[indx + v2]));
            float crl = cfa[indx - 1] * (dirwts1[indx - 2] + dirwts1[indx])
                        / (dirwts1[indx - 2] * (eps + cfa[indx]) + dirwts1[indx] * (eps + cfa[indx - 2]));
            float crr = cfa[indx + 1] * (dirwts1[indx + 2] + dirwts1[indx])
                        / (dirwts1[indx + 2] * (eps + cfa[indx]) + dirwts1[indx] * (eps + cfa[indx + 2]));

            // G interpolated in vert/hor directions using Hamilton-Adams method
            float guha = cfa[indx - v1] + xdiv2f(cfa[indx] - cfa[indx - v2]);
            float gdha = cfa[indx + v1] + xdiv2f(cfa[indx] - cfa[indx + v2]);
            float glha = cfa[indx - 1] + xdiv2f(cfa[indx] - cfa[indx - 2]);
            float grha = cfa[indx + 1] + xdiv2f(cfa[indx] - cfa[indx + 2]);

            // G interpolated in vert/hor directions using adaptive ratios
            float guar, gdar, glar, grar;

            if (fabsf(1.f - cru) < arthresh)
            {
              guar = cfa[indx] * cru;
            }
            else
            {
              guar = guha;
            }

            if (fabsf(1.f - crd) < arthresh)
            {
              gdar = cfa[indx] * crd;
            }
            else
            {
              gdar = gdha;
            }

            if (fabsf(1.f - crl) < arthresh)
            {
              glar = cfa[indx] * crl;
            }
            else
            {
              glar = glha;
            }

            if (fabsf(1.f - crr) < arthresh)
            {
              grar = cfa[indx] * crr;
            }
            else
            {
              grar = grha;
            }

            // adaptive weights for vertical/horizontal directions
            float hwt = dirwts1[indx - 1] / (dirwts1[indx - 1] + dirwts1[indx + 1]);
            float vwt = dirwts0[indx - v1] / (dirwts0[indx + v1] + dirwts0[indx - v1]);

            // interpolated G via adaptive weights of cardinal evaluations
            float Gintvha = vwt * gdha + (1.f - vwt) * guha;
            float Ginthha = hwt * grha + (1.f - hwt) * glha;

            // interpolated colour differences
            if (fcswitch)
            {
              vcd[indx]    = cfa[indx] - (vwt * gdar + (1.f - vwt) * guar);
              hcd[indx]    = cfa[indx] - (hwt * grar + (1.f - hwt) * glar);
              vcdalt[indx] = cfa[indx] - Gintvha;
              hcdalt[indx] = cfa[indx] - Ginthha;
            }
            else
            {
              // interpolated colour differences
              vcd[indx]    = (vwt * gdar + (1.f - vwt) * guar) - cfa[indx];
              hcd[indx]    = (hwt * grar + (1.f - hwt) * glar) - cfa[indx];
              vcdalt[indx] = Gintvha - cfa[indx];
              hcdalt[indx] = Ginthha - cfa[indx];
            }

            fcswitch = !fcswitch;

            if (cfa[indx] > clip_pt8 || Gintvha > clip_pt8 || Ginthha > clip_pt8)
            {
              // use HA if highlights are (nearly) clipped
              guar      = guha;
              gdar      = gdha;
              glar      = glha;
              grar      = grha;
              vcd[indx] = vcdalt[indx];
              hcd[indx] = hcdalt[indx];
            }

            // differences of interpolations in opposite directions
            dgintv[indx] = MIN(SQR(guha - gdha), SQR(guar - gdar));
            dginth[indx] = MIN(SQR(glha - grha), SQR(glar - grar));
          }
        }

#endif



#ifdef __SSE2__
        vfloat clip_ptv = F2V(clip_pt);
        vfloat sgn3v;

        if (!(FC(4, 4, filters) & 1))
        {
          sgnv = _mm_set_ps(1.f, -1.f, 1.f, -1.f);
        }
        else
        {
          sgnv = _mm_set_ps(-1.f, 1.f, -1.f, 1.f);
        }

        sgn3v = sgnv + sgnv + sgnv;

        for (int rr = 4; rr < rr1 - 4; rr++)
        {
          vfloat nsgnv = sgnv;
          sgnv         = -sgnv;
          sgn3v        = -sgn3v;

          for (int indx = rr * ts + 4; indx < rr * ts + cc1 - 4; indx += 4)
          {
            vfloat hcdv    = LVF(hcd[indx]);
            vfloat hcdvarv = SQRV(LVFU(hcd[indx - 2]) - hcdv) + SQRV(LVFU(hcd[indx - 2]) - LVFU(hcd[indx + 2]))
                             + SQRV(hcdv - LVFU(hcd[indx + 2]));
            vfloat hcdaltv    = LVF(hcdalt[indx]);
            vfloat hcdaltvarv = SQRV(LVFU(hcdalt[indx - 2]) - hcdaltv)
                                + SQRV(LVFU(hcdalt[indx - 2]) - LVFU(hcdalt[indx + 2]))
                                + SQRV(hcdaltv - LVFU(hcdalt[indx + 2]));
            vfloat vcdv    = LVF(vcd[indx]);
            vfloat vcdvarv = SQRV(LVF(vcd[indx - v2]) - vcdv) + SQRV(LVF(vcd[indx - v2]) - LVF(vcd[indx + v2]))
                             + SQRV(vcdv - LVF(vcd[indx + v2]));
            vfloat vcdaltv    = LVF(vcdalt[indx]);
            vfloat vcdaltvarv = SQRV(LVF(vcdalt[indx - v2]) - vcdaltv)
                                + SQRV(LVF(vcdalt[indx - v2]) - LVF(vcdalt[indx + v2]))
                                + SQRV(vcdaltv - LVF(vcdalt[indx + v2]));

            // choose the smallest variance; this yields a smoother interpolation
            hcdv = vself(vmaskf_lt(hcdaltvarv, hcdvarv), hcdaltv, hcdv);
            vcdv = vself(vmaskf_lt(vcdaltvarv, vcdvarv), vcdaltv, vcdv);

            // bound the interpolation in regions of high saturation
            // vertical and horizontal G interpolations
            vfloat Ginthv  = sgnv * hcdv + LVF(cfa[indx]);
            vfloat temp2v  = sgn3v * hcdv;
            vfloat hwtv    = onev + temp2v / (epsv + Ginthv + LVF(cfa[indx]));
            vmask  hcdmask = vmaskf_gt(nsgnv * hcdv, ZEROV);
            vfloat hcdoldv = hcdv;
            vfloat tempv   = nsgnv * (LVF(cfa[indx]) - ULIMV(Ginthv, LVFU(cfa[indx - 1]), LVFU(cfa[indx + 1])));
            hcdv           = vself(vmaskf_lt(temp2v, -(LVF(cfa[indx]) + Ginthv)), tempv, vintpf(hwtv, hcdv, tempv));
            hcdv           = vself(hcdmask, hcdv, hcdoldv);
            hcdv           = vself(vmaskf_gt(Ginthv, clip_ptv), tempv, hcdv);
            STVF(hcd[indx], hcdv);

            vfloat Gintvv  = sgnv * vcdv + LVF(cfa[indx]);
            temp2v         = sgn3v * vcdv;
            vfloat vwtv    = onev + temp2v / (epsv + Gintvv + LVF(cfa[indx]));
            vmask  vcdmask = vmaskf_gt(nsgnv * vcdv, ZEROV);
            vfloat vcdoldv = vcdv;
            tempv          = nsgnv * (LVF(cfa[indx]) - ULIMV(Gintvv, LVF(cfa[indx - v1]), LVF(cfa[indx + v1])));
            vcdv           = vself(vmaskf_lt(temp2v, -(LVF(cfa[indx]) + Gintvv)), tempv, vintpf(vwtv, vcdv, tempv));
            vcdv           = vself(vcdmask, vcdv, vcdoldv);
            vcdv           = vself(vmaskf_gt(Gintvv, clip_ptv), tempv, vcdv);
            STVF(vcd[indx], vcdv);
            STVFU(cddiffsq[indx], SQRV(vcdv - hcdv));
          }
        }

#else

        for (int rr = 4; rr < rr1 - 4; rr++)
        {
          for (int cc = 4, indx = rr * ts + cc, c = FC(rr, cc, filters) & 1; cc < cc1 - 4; cc++, indx++)
          {
            float hcdvar = 3.f * (SQR(hcd[indx - 2]) + SQR(hcd[indx]) + SQR(hcd[indx + 2]))
                           - SQR(hcd[indx - 2] + hcd[indx] + hcd[indx + 2]);
            float hcdaltvar = 3.f * (SQR(hcdalt[indx - 2]) + SQR(hcdalt[indx]) + SQR(hcdalt[indx + 2]))
                              - SQR(hcdalt[indx - 2] + hcdalt[indx] + hcdalt[indx + 2]);
            float vcdvar = 3.f * (SQR(vcd[indx - v2]) + SQR(vcd[indx]) + SQR(vcd[indx + v2]))
                           - SQR(vcd[indx - v2] + vcd[indx] + vcd[indx + v2]);
            float vcdaltvar = 3.f * (SQR(vcdalt[indx - v2]) + SQR(vcdalt[indx]) + SQR(vcdalt[indx + v2]))
                              - SQR(vcdalt[indx - v2] + vcdalt[indx] + vcdalt[indx + v2]);

            // choose the smallest variance; this yields a smoother interpolation
            if (hcdaltvar < hcdvar)
            {
              hcd[indx] = hcdalt[indx];
            }

            if (vcdaltvar < vcdvar)
            {
              vcd[indx] = vcdalt[indx];
            }

            // bound the interpolation in regions of high saturation
            // vertical and horizontal G interpolations
            float Gintv, Ginth;

            if (c)
            {                                   // G site
              Ginth = -hcd[indx] + cfa[indx];   // R or B
              Gintv = -vcd[indx] + cfa[indx];   // B or R

              if (hcd[indx] > 0)
              {
                if (3.f * hcd[indx] > (Ginth + cfa[indx]))
                {
                  hcd[indx] = -ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]) + cfa[indx];
                }
                else
                {
                  float hwt = 1.f - 3.f * hcd[indx] / (eps + Ginth + cfa[indx]);
                  hcd[indx] = hwt * hcd[indx] + (1.f - hwt) * (-ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]) + cfa[indx]);
                }
              }

              if (vcd[indx] > 0)
              {
                if (3.f * vcd[indx] > (Gintv + cfa[indx]))
                {
                  vcd[indx] = -ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]) + cfa[indx];
                }
                else
                {
                  float vwt = 1.f - 3.f * vcd[indx] / (eps + Gintv + cfa[indx]);
                  vcd[indx]
                      = vwt * vcd[indx] + (1.f - vwt) * (-ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]) + cfa[indx]);
                }
              }

              if (Ginth > clip_pt)
              {
                hcd[indx] = -ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]) + cfa[indx];
              }

              if (Gintv > clip_pt)
              {
                vcd[indx] = -ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]) + cfa[indx];
              }
            }
            else
            {   // R or B site

              Ginth = hcd[indx] + cfa[indx];   // interpolated G
              Gintv = vcd[indx] + cfa[indx];

              if (hcd[indx] < 0)
              {
                if (3.f * hcd[indx] < -(Ginth + cfa[indx]))
                {
                  hcd[indx] = ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]) - cfa[indx];
                }
                else
                {
                  float hwt = 1.f + 3.f * hcd[indx] / (eps + Ginth + cfa[indx]);
                  hcd[indx] = hwt * hcd[indx] + (1.f - hwt) * (ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]) - cfa[indx]);
                }
              }

              if (vcd[indx] < 0)
              {
                if (3.f * vcd[indx] < -(Gintv + cfa[indx]))
                {
                  vcd[indx] = ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]) - cfa[indx];
                }
                else
                {
                  float vwt = 1.f + 3.f * vcd[indx] / (eps + Gintv + cfa[indx]);
                  vcd[indx] = vwt * vcd[indx] + (1.f - vwt) * (ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]) - cfa[indx]);
                }
              }

              if (Ginth > clip_pt)
              {
                hcd[indx] = ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]) - cfa[indx];
              }

              if (Gintv > clip_pt)
              {
                vcd[indx] = ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]) - cfa[indx];
              }

              cddiffsq[indx] = SQR(vcd[indx] - hcd[indx]);
            }

            c = !c;
          }
        }

#endif



#ifdef __SSE2__
        vfloat epssqv = F2V(epssq);

        for (int rr = 6; rr < rr1 - 6; rr++)
        {
          for (int indx = rr * ts + 6 + (FC(rr, 2, filters) & 1); indx < rr * ts + cc1 - 6; indx += 8)
          {
            // compute colour difference variances in cardinal directions
            vfloat tempv      = LC2VFU(vcd[indx]);
            vfloat uavev      = tempv + LC2VFU(vcd[indx - v1]) + LC2VFU(vcd[indx - v2]) + LC2VFU(vcd[indx - v3]);
            vfloat davev      = tempv + LC2VFU(vcd[indx + v1]) + LC2VFU(vcd[indx + v2]) + LC2VFU(vcd[indx + v3]);
            vfloat Dgrbvvaruv = SQRV(tempv - uavev) + SQRV(LC2VFU(vcd[indx - v1]) - uavev)
                                + SQRV(LC2VFU(vcd[indx - v2]) - uavev) + SQRV(LC2VFU(vcd[indx - v3]) - uavev);
            vfloat Dgrbvvardv = SQRV(tempv - davev) + SQRV(LC2VFU(vcd[indx + v1]) - davev)
                                + SQRV(LC2VFU(vcd[indx + v2]) - davev) + SQRV(LC2VFU(vcd[indx + v3]) - davev);

            vfloat hwtv = vadivapb(LC2VFU(dirwts1[indx - 1]), LC2VFU(dirwts1[indx + 1]));
            vfloat vwtv = vadivapb(LC2VFU(dirwts0[indx - v1]), LC2VFU(dirwts0[indx + v1]));

            tempv        = LC2VFU(hcd[indx]);
            vfloat lavev = tempv + vaddc2vfu(hcd[indx - 3]) + LC2VFU(hcd[indx - 1]);
            vfloat ravev = tempv + vaddc2vfu(hcd[indx + 1]) + LC2VFU(hcd[indx + 3]);

            vfloat Dgrbhvarlv = SQRV(tempv - lavev) + SQRV(LC2VFU(hcd[indx - 1]) - lavev)
                                + SQRV(LC2VFU(hcd[indx - 2]) - lavev) + SQRV(LC2VFU(hcd[indx - 3]) - lavev);
            vfloat Dgrbhvarrv = SQRV(tempv - ravev) + SQRV(LC2VFU(hcd[indx + 1]) - ravev)
                                + SQRV(LC2VFU(hcd[indx + 2]) - ravev) + SQRV(LC2VFU(hcd[indx + 3]) - ravev);


            vfloat vcdvarv = epssqv + vintpf(vwtv, Dgrbvvardv, Dgrbvvaruv);
            vfloat hcdvarv = epssqv + vintpf(hwtv, Dgrbhvarrv, Dgrbhvarlv);

            // compute fluctuations in up/down and left/right interpolations of colours
            Dgrbvvaruv = LC2VFU(dgintv[indx - v1]) + LC2VFU(dgintv[indx - v2]);
            Dgrbvvardv = LC2VFU(dgintv[indx + v1]) + LC2VFU(dgintv[indx + v2]);

            Dgrbhvarlv = vaddc2vfu(dginth[indx - 2]);
            Dgrbhvarrv = vaddc2vfu(dginth[indx + 1]);

            vfloat vcdvar1v = epssqv + LC2VFU(dgintv[indx]) + vintpf(vwtv, Dgrbvvardv, Dgrbvvaruv);
            vfloat hcdvar1v = epssqv + LC2VFU(dginth[indx]) + vintpf(hwtv, Dgrbhvarrv, Dgrbhvarlv);

            // determine adaptive weights for G interpolation
            vfloat varwtv  = hcdvarv / (vcdvarv + hcdvarv);
            vfloat diffwtv = hcdvar1v / (vcdvar1v + hcdvar1v);

            // if both agree on interpolation direction, choose the one with strongest directional
            // discrimination;
            // otherwise, choose the u/d and l/r difference fluctuation weights
            vmask decmask = vandm(
                vmaskf_gt((zd5v - varwtv) * (zd5v - diffwtv), ZEROV),
                vmaskf_lt(vabsf(zd5v - diffwtv), vabsf(zd5v - varwtv)));
            STVFU(hvwt[indx >> 1], vself(decmask, varwtv, diffwtv));
          }
        }

#else

        for (int rr = 6; rr < rr1 - 6; rr++)
        {
          for (int cc = 6 + (FC(rr, 2, filters) & 1), indx = rr * ts + cc; cc < cc1 - 6; cc += 2, indx += 2)
          {
            // compute colour difference variances in cardinal directions

            float uave = vcd[indx] + vcd[indx - v1] + vcd[indx - v2] + vcd[indx - v3];
            float dave = vcd[indx] + vcd[indx + v1] + vcd[indx + v2] + vcd[indx + v3];
            float lave = hcd[indx] + hcd[indx - 1] + hcd[indx - 2] + hcd[indx - 3];
            float rave = hcd[indx] + hcd[indx + 1] + hcd[indx + 2] + hcd[indx + 3];

            // colour difference (G-R or G-B) variance in up/down/left/right directions
            float Dgrbvvaru = SQR(vcd[indx] - uave) + SQR(vcd[indx - v1] - uave) + SQR(vcd[indx - v2] - uave)
                              + SQR(vcd[indx - v3] - uave);
            float Dgrbvvard = SQR(vcd[indx] - dave) + SQR(vcd[indx + v1] - dave) + SQR(vcd[indx + v2] - dave)
                              + SQR(vcd[indx + v3] - dave);
            float Dgrbhvarl = SQR(hcd[indx] - lave) + SQR(hcd[indx - 1] - lave) + SQR(hcd[indx - 2] - lave)
                              + SQR(hcd[indx - 3] - lave);
            float Dgrbhvarr = SQR(hcd[indx] - rave) + SQR(hcd[indx + 1] - rave) + SQR(hcd[indx + 2] - rave)
                              + SQR(hcd[indx + 3] - rave);

            float hwt = dirwts1[indx - 1] / (dirwts1[indx - 1] + dirwts1[indx + 1]);
            float vwt = dirwts0[indx - v1] / (dirwts0[indx + v1] + dirwts0[indx - v1]);

            float vcdvar = epssq + vwt * Dgrbvvard + (1.f - vwt) * Dgrbvvaru;
            float hcdvar = epssq + hwt * Dgrbhvarr + (1.f - hwt) * Dgrbhvarl;

            // compute fluctuations in up/down and left/right interpolations of colours
            Dgrbvvaru = (dgintv[indx]) + (dgintv[indx - v1]) + (dgintv[indx - v2]);
            Dgrbvvard = (dgintv[indx]) + (dgintv[indx + v1]) + (dgintv[indx + v2]);
            Dgrbhvarl = (dginth[indx]) + (dginth[indx - 1]) + (dginth[indx - 2]);
            Dgrbhvarr = (dginth[indx]) + (dginth[indx + 1]) + (dginth[indx + 2]);

            float vcdvar1 = epssq + vwt * Dgrbvvard + (1.f - vwt) * Dgrbvvaru;
            float hcdvar1 = epssq + hwt * Dgrbhvarr + (1.f - hwt) * Dgrbhvarl;

            // determine adaptive weights for G interpolation
            float varwt  = hcdvar / (vcdvar + hcdvar);
            float diffwt = hcdvar1 / (vcdvar1 + hcdvar1);

            // if both agree on interpolation direction, choose the one with strongest directional
            // discrimination;
            // otherwise, choose the u/d and l/r difference fluctuation weights
            if ((0.5 - varwt) * (0.5 - diffwt) > 0 && fabsf(0.5f - diffwt) < fabsf(0.5f - varwt))
            {
              hvwt[indx >> 1] = varwt;
            }
            else
            {
              hvwt[indx >> 1] = diffwt;
            }
          }
        }

#endif

#ifdef __SSE2__
        vfloat gaussg0 = F2V(gaussgrad[0]);
        vfloat gaussg1 = F2V(gaussgrad[1]);
        vfloat gaussg2 = F2V(gaussgrad[2]);
        vfloat gaussg3 = F2V(gaussgrad[3]);
        vfloat gaussg4 = F2V(gaussgrad[4]);
        vfloat gaussg5 = F2V(gaussgrad[5]);
        vfloat gausso0 = F2V(gaussodd[0]);
        vfloat gausso1 = F2V(gaussodd[1]);
        vfloat gausso2 = F2V(gaussodd[2]);
        vfloat gausso3 = F2V(gaussodd[3]);

#endif

        // precompute nyquist
        for (int rr = 6; rr < rr1 - 6; rr++)
        {
          int cc   = 6 + (FC(rr, 2, filters) & 1);
          int indx = rr * ts + cc;

#ifdef __SSE2__

          for (; cc < cc1 - 7; cc += 8, indx += 8)
          {
            vfloat valv = (gausso0 * LC2VFU(cddiffsq[indx])
                           + gausso1
                                 * (LC2VFU(cddiffsq[(indx - m1)]) + LC2VFU(cddiffsq[(indx + p1)])
                                    + LC2VFU(cddiffsq[(indx - p1)]) + LC2VFU(cddiffsq[(indx + m1)]))
                           + gausso2
                                 * (LC2VFU(cddiffsq[(indx - v2)]) + LC2VFU(cddiffsq[(indx - 2)])
                                    + LC2VFU(cddiffsq[(indx + 2)]) + LC2VFU(cddiffsq[(indx + v2)]))
                           + gausso3
                                 * (LC2VFU(cddiffsq[(indx - m2)]) + LC2VFU(cddiffsq[(indx + p2)])
                                    + LC2VFU(cddiffsq[(indx - p2)]) + LC2VFU(cddiffsq[(indx + m2)])))
                          - (gaussg0 * LC2VFU(delhvsqsum[indx])
                             + gaussg1
                                   * (LC2VFU(delhvsqsum[indx - v1]) + LC2VFU(delhvsqsum[indx - 1])
                                      + LC2VFU(delhvsqsum[indx + 1]) + LC2VFU(delhvsqsum[indx + v1]))
                             + gaussg2
                                   * (LC2VFU(delhvsqsum[indx - m1]) + LC2VFU(delhvsqsum[indx + p1])
                                      + LC2VFU(delhvsqsum[indx - p1]) + LC2VFU(delhvsqsum[indx + m1]))
                             + gaussg3
                                   * (LC2VFU(delhvsqsum[indx - v2]) + LC2VFU(delhvsqsum[indx - 2])
                                      + LC2VFU(delhvsqsum[indx + 2]) + LC2VFU(delhvsqsum[indx + v2]))
                             + gaussg4
                                   * (LC2VFU(delhvsqsum[indx - v2 - 1]) + LC2VFU(delhvsqsum[indx - v2 + 1])
                                      + LC2VFU(delhvsqsum[indx - ts - 2]) + LC2VFU(delhvsqsum[indx - ts + 2])
                                      + LC2VFU(delhvsqsum[indx + ts - 2]) + LC2VFU(delhvsqsum[indx + ts + 2])
                                      + LC2VFU(delhvsqsum[indx + v2 - 1]) + LC2VFU(delhvsqsum[indx + v2 + 1]))
                             + gaussg5
                                   * (LC2VFU(delhvsqsum[indx - m2]) + LC2VFU(delhvsqsum[indx + p2])
                                      + LC2VFU(delhvsqsum[indx - p2]) + LC2VFU(delhvsqsum[indx + m2])));
            STVFU(nyqutest[indx >> 1], valv);
          }

#endif

          for (; cc < cc1 - 6; cc += 2, indx += 2)
          {
            nyqutest[indx >> 1]
                = (gaussodd[0] * cddiffsq[indx]
                   + gaussodd[1]
                         * (cddiffsq[(indx - m1)] + cddiffsq[(indx + p1)] + cddiffsq[(indx - p1)]
                            + cddiffsq[(indx + m1)])
                   + gaussodd[2]
                         * (cddiffsq[(indx - v2)] + cddiffsq[(indx - 2)] + cddiffsq[(indx + 2)] + cddiffsq[(indx + v2)])
                   + gaussodd[3]
                         * (cddiffsq[(indx - m2)] + cddiffsq[(indx + p2)] + cddiffsq[(indx - p2)]
                            + cddiffsq[(indx + m2)]))
                  - (gaussgrad[0] * delhvsqsum[indx]
                     + gaussgrad[1]
                           * (delhvsqsum[indx - v1] + delhvsqsum[indx + 1] + delhvsqsum[indx - 1]
                              + delhvsqsum[indx + v1])
                     + gaussgrad[2]
                           * (delhvsqsum[indx - m1] + delhvsqsum[indx + p1] + delhvsqsum[indx - p1]
                              + delhvsqsum[indx + m1])
                     + gaussgrad[3]
                           * (delhvsqsum[indx - v2] + delhvsqsum[indx - 2] + delhvsqsum[indx + 2]
                              + delhvsqsum[indx + v2])
                     + gaussgrad[4]
                           * (delhvsqsum[indx - v2 - 1] + delhvsqsum[indx - v2 + 1] + delhvsqsum[indx - ts - 2]
                              + delhvsqsum[indx - ts + 2] + delhvsqsum[indx + ts - 2] + delhvsqsum[indx + ts + 2]
                              + delhvsqsum[indx + v2 - 1] + delhvsqsum[indx + v2 + 1])
                     + gaussgrad[5]
                           * (delhvsqsum[indx - m2] + delhvsqsum[indx + p2] + delhvsqsum[indx - p2]
                              + delhvsqsum[indx + m2]));
          }
        }

        // Nyquist test
        int nystartrow = 0;
        int nyendrow   = 0;
        int nystartcol = ts + 1;
        int nyendcol   = 0;

        for (int rr = 6; rr < rr1 - 6; rr++)
        {
          for (int cc = 6 + (FC(rr, 2, filters) & 1), indx = rr * ts + cc; cc < cc1 - 6; cc += 2, indx += 2)
          {
            // nyquist texture test: ask if difference of vcd compared to hcd is larger or smaller than RGGB
            // gradients
            if (nyqutest[indx >> 1] > 0.f)
            {
              nyquist[indx >> 1] = 1;   // nyquist=1 for nyquist region
              nystartrow         = nystartrow ? nystartrow : rr;
              nyendrow           = rr;
              nystartcol         = nystartcol > cc ? cc : nystartcol;
              nyendcol           = nyendcol < cc ? cc : nyendcol;
            }
          }
        }


        bool doNyquist = nystartrow != nyendrow && nystartcol != nyendcol;

        if (doNyquist)
        {
          nyendrow++;   // because of < condition
          nyendcol++;   // because of < condition
          nystartcol -= (nystartcol & 1);
          nystartrow = std::max(8, nystartrow);
          nyendrow   = std::min(rr1 - 8, nyendrow);
          nystartcol = std::max(8, nystartcol);
          nyendcol   = std::min(cc1 - 8, nyendcol);
          memset(&nyquist2[4 * tsh], 0, sizeof(char) * (ts - 8) * tsh);

#ifdef __SSE2__
          vint fourvb = _mm_set1_epi8(4);
          vint onevb  = _mm_set1_epi8(1);

#endif

          for (int rr = nystartrow; rr < nyendrow; rr++)
          {
#ifdef __SSE2__

            for (int indx = rr * ts; indx < rr * ts + cc1; indx += 32)
            {
              vint nyquisttemp1v = _mm_adds_epi8(
                  _mm_load_si128((vint *)&nyquist[(indx - v2) >> 1]),
                  _mm_loadu_si128((vint *)&nyquist[(indx - m1) >> 1]));
              vint nyquisttemp2v = _mm_adds_epi8(
                  _mm_loadu_si128((vint *)&nyquist[(indx + p1) >> 1]),
                  _mm_loadu_si128((vint *)&nyquist[(indx - 2) >> 1]));
              vint nyquisttemp3v = _mm_adds_epi8(
                  _mm_loadu_si128((vint *)&nyquist[(indx + 2) >> 1]),
                  _mm_loadu_si128((vint *)&nyquist[(indx - p1) >> 1]));
              vint valv          = _mm_load_si128((vint *)&nyquist[indx >> 1]);
              vint nyquisttemp4v = _mm_adds_epi8(
                  _mm_loadu_si128((vint *)&nyquist[(indx + m1) >> 1]),
                  _mm_load_si128((vint *)&nyquist[(indx + v2) >> 1]));
              nyquisttemp1v = _mm_adds_epi8(nyquisttemp1v, nyquisttemp3v);
              nyquisttemp2v = _mm_adds_epi8(nyquisttemp2v, nyquisttemp4v);
              nyquisttemp1v = _mm_adds_epi8(nyquisttemp1v, nyquisttemp2v);
              valv          = vselc(_mm_cmpgt_epi8(nyquisttemp1v, fourvb), onevb, valv);
              valv          = vselinotzero(_mm_cmplt_epi8(nyquisttemp1v, fourvb), valv);
              _mm_store_si128((vint *)&nyquist2[indx >> 1], valv);
            }

#else

            for (int indx = rr * ts + nystartcol + (FC(rr, 2, filters) & 1); indx < rr * ts + nyendcol; indx += 2)
            {
              unsigned int nyquisttemp
                  = (nyquist[(indx - v2) >> 1] + nyquist[(indx - m1) >> 1] + nyquist[(indx + p1) >> 1]
                     + nyquist[(indx - 2) >> 1] + nyquist[(indx + 2) >> 1] + nyquist[(indx - p1) >> 1]
                     + nyquist[(indx + m1) >> 1] + nyquist[(indx + v2) >> 1]);
              // if most of your neighbours are named Nyquist, it's likely that you're one too, or not
              nyquist2[indx >> 1] = nyquisttemp > 4 ? 1 : (nyquisttemp < 4 ? 0 : nyquist[indx >> 1]);
            }

#endif
          }

          // end of Nyquist test

          // in areas of Nyquist texture, do area interpolation
          for (int rr = nystartrow; rr < nyendrow; rr++)
            for (int indx = rr * ts + nystartcol + (FC(rr, 2, filters) & 1); indx < rr * ts + nyendcol; indx += 2)
            {
              if (nyquist2[indx >> 1])
              {
                // area interpolation

                float sumcfa = 0.f, sumh = 0.f, sumv = 0.f, sumsqh = 0.f, sumsqv = 0.f, areawt = 0.f;

                for (int i = -6; i < 7; i += 2)
                {
                  int indx1 = indx + (i * ts) - 6;

                  for (int j = -6; j < 7; j += 2, indx1 += 2)
                  {
                    if (nyquist2[indx1 >> 1])
                    {
                      float cfatemp = cfa[indx1];
                      sumcfa += cfatemp;
                      sumh += (cfa[indx1 - 1] + cfa[indx1 + 1]);
                      sumv += (cfa[indx1 - v1] + cfa[indx1 + v1]);
                      sumsqh += SQR(cfatemp - cfa[indx1 - 1]) + SQR(cfatemp - cfa[indx1 + 1]);
                      sumsqv += SQR(cfatemp - cfa[indx1 - v1]) + SQR(cfatemp - cfa[indx1 + v1]);
                      areawt += 1;
                    }
                  }
                }

                // horizontal and vertical colour differences, and adaptive weight
                sumh            = sumcfa - xdiv2f(sumh);
                sumv            = sumcfa - xdiv2f(sumv);
                areawt          = xdiv2f(areawt);
                float hcdvar    = epssq + fabsf(areawt * sumsqh - sumh * sumh);
                float vcdvar    = epssq + fabsf(areawt * sumsqv - sumv * sumv);
                hvwt[indx >> 1] = hcdvar / (vcdvar + hcdvar);

                // end of area interpolation
              }
            }
        }


        // populate G at R/B sites
        for (int rr = 8; rr < rr1 - 8; rr++)
          for (int indx = rr * ts + 8 + (FC(rr, 2, filters) & 1); indx < rr * ts + cc1 - 8; indx += 2)
          {
            // first ask if one gets more directional discrimination from nearby B/R sites
            float hvwtalt = xdivf(
                hvwt[(indx - m1) >> 1] + hvwt[(indx + p1) >> 1] + hvwt[(indx - p1) >> 1] + hvwt[(indx + m1) >> 1],
                2);

            hvwt[indx >> 1] = fabsf(0.5f - hvwt[indx >> 1]) < fabsf(0.5f - hvwtalt) ? hvwtalt : hvwt[indx >> 1];
            // a better result was obtained from the neighbours

            Dgrb[0][indx >> 1] = intp(hvwt[indx >> 1], vcd[indx], hcd[indx]);   // evaluate colour differences

            rgbgreen[indx] = cfa[indx] + Dgrb[0][indx >> 1];   // evaluate G (finally!)

            // local curvature in G (preparation for nyquist refinement step)
            Dgrb2[indx >> 1].h
                = nyquist2[indx >> 1] ? SQR(rgbgreen[indx] - xdiv2f(rgbgreen[indx - 1] + rgbgreen[indx + 1])) : 0.f;
            Dgrb2[indx >> 1].v
                = nyquist2[indx >> 1] ? SQR(rgbgreen[indx] - xdiv2f(rgbgreen[indx - v1] + rgbgreen[indx + v1])) : 0.f;
          }


        // end of standard interpolation

        // refine Nyquist areas using G curvatures
        if (doNyquist)
        {
          for (int rr = nystartrow; rr < nyendrow; rr++)
            for (int indx = rr * ts + nystartcol + (FC(rr, 2, filters) & 1); indx < rr * ts + nyendcol; indx += 2)
            {
              if (nyquist2[indx >> 1])
              {
                // local averages (over Nyquist pixels only) of G curvature squared
                float gvarh = epssq
                              + (gquinc[0] * Dgrb2[indx >> 1].h
                                 + gquinc[1]
                                       * (Dgrb2[(indx - m1) >> 1].h + Dgrb2[(indx + p1) >> 1].h
                                          + Dgrb2[(indx - p1) >> 1].h + Dgrb2[(indx + m1) >> 1].h)
                                 + gquinc[2]
                                       * (Dgrb2[(indx - v2) >> 1].h + Dgrb2[(indx - 2) >> 1].h
                                          + Dgrb2[(indx + 2) >> 1].h + Dgrb2[(indx + v2) >> 1].h)
                                 + gquinc[3]
                                       * (Dgrb2[(indx - m2) >> 1].h + Dgrb2[(indx + p2) >> 1].h
                                          + Dgrb2[(indx - p2) >> 1].h + Dgrb2[(indx + m2) >> 1].h));
                float gvarv = epssq
                              + (gquinc[0] * Dgrb2[indx >> 1].v
                                 + gquinc[1]
                                       * (Dgrb2[(indx - m1) >> 1].v + Dgrb2[(indx + p1) >> 1].v
                                          + Dgrb2[(indx - p1) >> 1].v + Dgrb2[(indx + m1) >> 1].v)
                                 + gquinc[2]
                                       * (Dgrb2[(indx - v2) >> 1].v + Dgrb2[(indx - 2) >> 1].v
                                          + Dgrb2[(indx + 2) >> 1].v + Dgrb2[(indx + v2) >> 1].v)
                                 + gquinc[3]
                                       * (Dgrb2[(indx - m2) >> 1].v + Dgrb2[(indx + p2) >> 1].v
                                          + Dgrb2[(indx - p2) >> 1].v + Dgrb2[(indx + m2) >> 1].v));
                // use the results as weights for refined G interpolation
                Dgrb[0][indx >> 1] = (hcd[indx] * gvarv + vcd[indx] * gvarh) / (gvarv + gvarh);
                rgbgreen[indx]     = cfa[indx] + Dgrb[0][indx >> 1];
              }
            }
        }


#ifdef __SSE2__

        for (int rr = 6; rr < rr1 - 6; rr++)
        {
          if ((FC(rr, 2, filters) & 1) == 0)
          {
            for (int cc = 6, indx = rr * ts + cc; cc < cc1 - 6; cc += 8, indx += 8)
            {
              vfloat tempv     = LC2VFU(cfa[indx + 1]);
              vfloat Dgrbsq1pv = (SQRV(tempv - LC2VFU(cfa[indx + 1 - p1])) + SQRV(tempv - LC2VFU(cfa[indx + 1 + p1])));
              STVFU(delp[indx >> 1], vabsf(LC2VFU(cfa[indx + p1]) - LC2VFU(cfa[indx - p1])));
              STVFU(delm[indx >> 1], vabsf(LC2VFU(cfa[indx + m1]) - LC2VFU(cfa[indx - m1])));
              vfloat Dgrbsq1mv = (SQRV(tempv - LC2VFU(cfa[indx + 1 - m1])) + SQRV(tempv - LC2VFU(cfa[indx + 1 + m1])));
              STVFU(Dgrbsq1m[indx >> 1], Dgrbsq1mv);
              STVFU(Dgrbsq1p[indx >> 1], Dgrbsq1pv);
            }
          }
          else
          {
            for (int cc = 6, indx = rr * ts + cc; cc < cc1 - 6; cc += 8, indx += 8)
            {
              vfloat tempv     = LC2VFU(cfa[indx]);
              vfloat Dgrbsq1pv = (SQRV(tempv - LC2VFU(cfa[indx - p1])) + SQRV(tempv - LC2VFU(cfa[indx + p1])));
              STVFU(delp[indx >> 1], vabsf(LC2VFU(cfa[indx + 1 + p1]) - LC2VFU(cfa[indx + 1 - p1])));
              STVFU(delm[indx >> 1], vabsf(LC2VFU(cfa[indx + 1 + m1]) - LC2VFU(cfa[indx + 1 - m1])));
              vfloat Dgrbsq1mv = (SQRV(tempv - LC2VFU(cfa[indx - m1])) + SQRV(tempv - LC2VFU(cfa[indx + m1])));
              STVFU(Dgrbsq1m[indx >> 1], Dgrbsq1mv);
              STVFU(Dgrbsq1p[indx >> 1], Dgrbsq1pv);
            }
          }
        }

#else

        for (int rr = 6; rr < rr1 - 6; rr++)
        {
          if ((FC(rr, 2, filters) & 1) == 0)
          {
            for (int cc = 6, indx = rr * ts + cc; cc < cc1 - 6; cc += 2, indx += 2)
            {
              delp[indx >> 1]     = fabsf(cfa[indx + p1] - cfa[indx - p1]);
              delm[indx >> 1]     = fabsf(cfa[indx + m1] - cfa[indx - m1]);
              Dgrbsq1p[indx >> 1] = (SQR(cfa[indx + 1] - cfa[indx + 1 - p1]) + SQR(cfa[indx + 1] - cfa[indx + 1 + p1]));
              Dgrbsq1m[indx >> 1] = (SQR(cfa[indx + 1] - cfa[indx + 1 - m1]) + SQR(cfa[indx + 1] - cfa[indx + 1 + m1]));
            }
          }
          else
          {
            for (int cc = 6, indx = rr * ts + cc; cc < cc1 - 6; cc += 2, indx += 2)
            {
              Dgrbsq1p[indx >> 1] = (SQR(cfa[indx] - cfa[indx - p1]) + SQR(cfa[indx] - cfa[indx + p1]));
              Dgrbsq1m[indx >> 1] = (SQR(cfa[indx] - cfa[indx - m1]) + SQR(cfa[indx] - cfa[indx + m1]));
              delp[indx >> 1]     = fabsf(cfa[indx + 1 + p1] - cfa[indx + 1 - p1]);
              delm[indx >> 1]     = fabsf(cfa[indx + 1 + m1] - cfa[indx + 1 - m1]);
            }
          }
        }

#endif

        // diagonal interpolation correction

#ifdef __SSE2__
        vfloat gausseven0v = F2V(gausseven[0]);
        vfloat gausseven1v = F2V(gausseven[1]);
#endif

        for (int rr = 8; rr < rr1 - 8; rr++)
        {
#ifdef __SSE2__

          for (int indx = rr * ts + 8 + (FC(rr, 2, filters) & 1), indx1 = indx >> 1; indx < rr * ts + cc1 - 8;
               indx += 8, indx1 += 4)
          {
            // diagonal colour ratios
            vfloat cfav = LC2VFU(cfa[indx]);

            vfloat temp1v = LC2VFU(cfa[indx + m1]);
            vfloat temp2v = LC2VFU(cfa[indx + m2]);
            vfloat rbsev  = vmul2f(temp1v) / (epsv + cfav + temp2v);
            rbsev = vself(vmaskf_lt(vabsf(onev - rbsev), arthreshv), cfav * rbsev, temp1v + zd5v * (cfav - temp2v));

            temp1v       = LC2VFU(cfa[indx - m1]);
            temp2v       = LC2VFU(cfa[indx - m2]);
            vfloat rbnwv = vmul2f(temp1v) / (epsv + cfav + temp2v);
            rbnwv = vself(vmaskf_lt(vabsf(onev - rbnwv), arthreshv), cfav * rbnwv, temp1v + zd5v * (cfav - temp2v));

            temp1v = epsv + LVFU(delm[indx1]);
            vfloat wtsev
                = temp1v + LVFU(delm[(indx + m1) >> 1]) + LVFU(delm[(indx + m2) >> 1]);   // same as for wtu,wtd,wtl,wtr
            vfloat wtnwv = temp1v + LVFU(delm[(indx - m1) >> 1]) + LVFU(delm[(indx - m2) >> 1]);

            vfloat rbmv = (wtsev * rbnwv + wtnwv * rbsev) / (wtsev + wtnwv);

            temp1v     = ULIMV(rbmv, LC2VFU(cfa[indx - m1]), LC2VFU(cfa[indx + m1]));
            vfloat wtv = vmul2f(cfav - rbmv) / (epsv + rbmv + cfav);
            temp2v     = vintpf(wtv, rbmv, temp1v);

            temp2v = vself(vmaskf_lt(rbmv + rbmv, cfav), temp1v, temp2v);
            temp2v = vself(vmaskf_lt(rbmv, cfav), temp2v, rbmv);
            STVFU(
                rbm[indx1],
                vself(
                    vmaskf_gt(temp2v, clip_ptv),
                    ULIMV(temp2v, LC2VFU(cfa[indx - m1]), LC2VFU(cfa[indx + m1])),
                    temp2v));


            temp1v       = LC2VFU(cfa[indx + p1]);
            temp2v       = LC2VFU(cfa[indx + p2]);
            vfloat rbnev = vmul2f(temp1v) / (epsv + cfav + temp2v);
            rbnev = vself(vmaskf_lt(vabsf(onev - rbnev), arthreshv), cfav * rbnev, temp1v + zd5v * (cfav - temp2v));

            temp1v       = LC2VFU(cfa[indx - p1]);
            temp2v       = LC2VFU(cfa[indx - p2]);
            vfloat rbswv = vmul2f(temp1v) / (epsv + cfav + temp2v);
            rbswv = vself(vmaskf_lt(vabsf(onev - rbswv), arthreshv), cfav * rbswv, temp1v + zd5v * (cfav - temp2v));

            temp1v       = epsv + LVFU(delp[indx1]);
            vfloat wtnev = temp1v + LVFU(delp[(indx + p1) >> 1]) + LVFU(delp[(indx + p2) >> 1]);
            vfloat wtswv = temp1v + LVFU(delp[(indx - p1) >> 1]) + LVFU(delp[(indx - p2) >> 1]);

            vfloat rbpv = (wtnev * rbswv + wtswv * rbnev) / (wtnev + wtswv);

            temp1v = ULIMV(rbpv, LC2VFU(cfa[indx - p1]), LC2VFU(cfa[indx + p1]));
            wtv    = vmul2f(cfav - rbpv) / (epsv + rbpv + cfav);
            temp2v = vintpf(wtv, rbpv, temp1v);

            temp2v = vself(vmaskf_lt(rbpv + rbpv, cfav), temp1v, temp2v);
            temp2v = vself(vmaskf_lt(rbpv, cfav), temp2v, rbpv);
            STVFU(
                rbp[indx1],
                vself(
                    vmaskf_gt(temp2v, clip_ptv),
                    ULIMV(temp2v, LC2VFU(cfa[indx - p1]), LC2VFU(cfa[indx + p1])),
                    temp2v));

            vfloat rbvarmv
                = epssqv
                  + (gausseven0v
                         * (LVFU(Dgrbsq1m[(indx - v1) >> 1]) + LVFU(Dgrbsq1m[(indx - 1) >> 1])
                            + LVFU(Dgrbsq1m[(indx + 1) >> 1]) + LVFU(Dgrbsq1m[(indx + v1) >> 1]))
                     + gausseven1v
                           * (LVFU(Dgrbsq1m[(indx - v2 - 1) >> 1]) + LVFU(Dgrbsq1m[(indx - v2 + 1) >> 1])
                              + LVFU(Dgrbsq1m[(indx - 2 - v1) >> 1]) + LVFU(Dgrbsq1m[(indx + 2 - v1) >> 1])
                              + LVFU(Dgrbsq1m[(indx - 2 + v1) >> 1]) + LVFU(Dgrbsq1m[(indx + 2 + v1) >> 1])
                              + LVFU(Dgrbsq1m[(indx + v2 - 1) >> 1]) + LVFU(Dgrbsq1m[(indx + v2 + 1) >> 1])));
            STVFU(
                pmwt[indx1],
                rbvarmv
                    / ((epssqv
                        + (gausseven0v
                               * (LVFU(Dgrbsq1p[(indx - v1) >> 1]) + LVFU(Dgrbsq1p[(indx - 1) >> 1])
                                  + LVFU(Dgrbsq1p[(indx + 1) >> 1]) + LVFU(Dgrbsq1p[(indx + v1) >> 1]))
                           + gausseven1v
                                 * (LVFU(Dgrbsq1p[(indx - v2 - 1) >> 1]) + LVFU(Dgrbsq1p[(indx - v2 + 1) >> 1])
                                    + LVFU(Dgrbsq1p[(indx - 2 - v1) >> 1]) + LVFU(Dgrbsq1p[(indx + 2 - v1) >> 1])
                                    + LVFU(Dgrbsq1p[(indx - 2 + v1) >> 1]) + LVFU(Dgrbsq1p[(indx + 2 + v1) >> 1])
                                    + LVFU(Dgrbsq1p[(indx + v2 - 1) >> 1]) + LVFU(Dgrbsq1p[(indx + v2 + 1) >> 1]))))
                       + rbvarmv));
          }

#else

          for (int cc = 8 + (FC(rr, 2, filters) & 1), indx = rr * ts + cc, indx1 = indx >> 1; cc < cc1 - 8;
               cc += 2, indx += 2, indx1++)
          {
            // diagonal colour ratios
            float crse = xmul2f(cfa[indx + m1]) / (eps + cfa[indx] + (cfa[indx + m2]));
            float crnw = xmul2f(cfa[indx - m1]) / (eps + cfa[indx] + (cfa[indx - m2]));
            float crne = xmul2f(cfa[indx + p1]) / (eps + cfa[indx] + (cfa[indx + p2]));
            float crsw = xmul2f(cfa[indx - p1]) / (eps + cfa[indx] + (cfa[indx - p2]));
            // colour differences in diagonal directions
            float rbse, rbnw, rbne, rbsw;

            // assign B/R at R/B sites
            if (fabsf(1.f - crse) < arthresh)
            {
              rbse = cfa[indx] * crse;   // use this if more precise diag interp is necessary
            }
            else
            {
              rbse = (cfa[indx + m1]) + xdiv2f(cfa[indx] - cfa[indx + m2]);
            }

            if (fabsf(1.f - crnw) < arthresh)
            {
              rbnw = cfa[indx] * crnw;
            }
            else
            {
              rbnw = (cfa[indx - m1]) + xdiv2f(cfa[indx] - cfa[indx - m2]);
            }

            if (fabsf(1.f - crne) < arthresh)
            {
              rbne = cfa[indx] * crne;
            }
            else
            {
              rbne = (cfa[indx + p1]) + xdiv2f(cfa[indx] - cfa[indx + p2]);
            }

            if (fabsf(1.f - crsw) < arthresh)
            {
              rbsw = cfa[indx] * crsw;
            }
            else
            {
              rbsw = (cfa[indx - p1]) + xdiv2f(cfa[indx] - cfa[indx - p2]);
            }

            float wtse
                = eps + delm[indx1] + delm[(indx + m1) >> 1] + delm[(indx + m2) >> 1];   // same as for wtu,wtd,wtl,wtr
            float wtnw = eps + delm[indx1] + delm[(indx - m1) >> 1] + delm[(indx - m2) >> 1];
            float wtne = eps + delp[indx1] + delp[(indx + p1) >> 1] + delp[(indx + p2) >> 1];
            float wtsw = eps + delp[indx1] + delp[(indx - p1) >> 1] + delp[(indx - p2) >> 1];


            rbm[indx1] = (wtse * rbnw + wtnw * rbse) / (wtse + wtnw);
            rbp[indx1] = (wtne * rbsw + wtsw * rbne) / (wtne + wtsw);

            // variance of R-B in plus/minus directions
            float rbvarm = epssq
                           + (gausseven[0]
                                  * (Dgrbsq1m[(indx - v1) >> 1] + Dgrbsq1m[(indx - 1) >> 1] + Dgrbsq1m[(indx + 1) >> 1]
                                     + Dgrbsq1m[(indx + v1) >> 1])
                              + gausseven[1]
                                    * (Dgrbsq1m[(indx - v2 - 1) >> 1] + Dgrbsq1m[(indx - v2 + 1) >> 1]
                                       + Dgrbsq1m[(indx - 2 - v1) >> 1] + Dgrbsq1m[(indx + 2 - v1) >> 1]
                                       + Dgrbsq1m[(indx - 2 + v1) >> 1] + Dgrbsq1m[(indx + 2 + v1) >> 1]
                                       + Dgrbsq1m[(indx + v2 - 1) >> 1] + Dgrbsq1m[(indx + v2 + 1) >> 1]));
            pmwt[indx1] = rbvarm
                          / ((epssq
                              + (gausseven[0]
                                     * (Dgrbsq1p[(indx - v1) >> 1] + Dgrbsq1p[(indx - 1) >> 1]
                                        + Dgrbsq1p[(indx + 1) >> 1] + Dgrbsq1p[(indx + v1) >> 1])
                                 + gausseven[1]
                                       * (Dgrbsq1p[(indx - v2 - 1) >> 1] + Dgrbsq1p[(indx - v2 + 1) >> 1]
                                          + Dgrbsq1p[(indx - 2 - v1) >> 1] + Dgrbsq1p[(indx + 2 - v1) >> 1]
                                          + Dgrbsq1p[(indx - 2 + v1) >> 1] + Dgrbsq1p[(indx + 2 + v1) >> 1]
                                          + Dgrbsq1p[(indx + v2 - 1) >> 1] + Dgrbsq1p[(indx + v2 + 1) >> 1])))
                             + rbvarm);

            // bound the interpolation in regions of high saturation

            if (rbp[indx1] < cfa[indx])
            {
              if (xmul2f(rbp[indx1]) < cfa[indx])
              {
                rbp[indx1] = ULIM(rbp[indx1], cfa[indx - p1], cfa[indx + p1]);
              }
              else
              {
                float pwt  = xmul2f(cfa[indx] - rbp[indx1]) / (eps + rbp[indx1] + cfa[indx]);
                rbp[indx1] = pwt * rbp[indx1] + (1.f - pwt) * ULIM(rbp[indx1], cfa[indx - p1], cfa[indx + p1]);
              }
            }

            if (rbm[indx1] < cfa[indx])
            {
              if (xmul2f(rbm[indx1]) < cfa[indx])
              {
                rbm[indx1] = ULIM(rbm[indx1], cfa[indx - m1], cfa[indx + m1]);
              }
              else
              {
                float mwt  = xmul2f(cfa[indx] - rbm[indx1]) / (eps + rbm[indx1] + cfa[indx]);
                rbm[indx1] = mwt * rbm[indx1] + (1.f - mwt) * ULIM(rbm[indx1], cfa[indx - m1], cfa[indx + m1]);
              }
            }

            if (rbp[indx1] > clip_pt)
            {
              rbp[indx1] = ULIM(rbp[indx1], cfa[indx - p1], cfa[indx + p1]);
            }

            if (rbm[indx1] > clip_pt)
            {
              rbm[indx1] = ULIM(rbm[indx1], cfa[indx - m1], cfa[indx + m1]);
            }
          }

#endif
        }

#ifdef __SSE2__
        vfloat zd25v = F2V(0.25f);
#endif

        for (int rr = 10; rr < rr1 - 10; rr++)
#ifdef __SSE2__
          for (int indx = rr * ts + 10 + (FC(rr, 2, filters) & 1), indx1 = indx >> 1; indx < rr * ts + cc1 - 10;
               indx += 8, indx1 += 4)
          {
            // first ask if one gets more directional discrimination from nearby B/R sites
            vfloat pmwtaltv = zd25v
                              * (LVFU(pmwt[(indx - m1) >> 1]) + LVFU(pmwt[(indx + p1) >> 1])
                                 + LVFU(pmwt[(indx - p1) >> 1]) + LVFU(pmwt[(indx + m1) >> 1]));
            vfloat tempv = LVFU(pmwt[indx1]);
            tempv        = vself(vmaskf_lt(vabsf(zd5v - tempv), vabsf(zd5v - pmwtaltv)), pmwtaltv, tempv);
            STVFU(pmwt[indx1], tempv);
            STVFU(rbint[indx1], zd5v * (LC2VFU(cfa[indx]) + vintpf(tempv, LVFU(rbp[indx1]), LVFU(rbm[indx1]))));
          }

#else

          for (int cc = 10 + (FC(rr, 2, filters) & 1), indx = rr * ts + cc, indx1 = indx >> 1; cc < cc1 - 10;
               cc += 2, indx += 2, indx1++)
          {
            // first ask if one gets more directional discrimination from nearby B/R sites
            float pmwtalt = xdivf(
                pmwt[(indx - m1) >> 1] + pmwt[(indx + p1) >> 1] + pmwt[(indx - p1) >> 1] + pmwt[(indx + m1) >> 1],
                2);

            if (fabsf(0.5f - pmwt[indx1]) < fabsf(0.5f - pmwtalt))
            {
              pmwt[indx1] = pmwtalt;   // a better result was obtained from the neighbours
            }

            rbint[indx1] = xdiv2f(
                cfa[indx] + rbm[indx1] * (1.f - pmwt[indx1]) + rbp[indx1] * pmwt[indx1]);   // this is R+B, interpolated
          }

#endif

        for (int rr = 12; rr < rr1 - 12; rr++)
#ifdef __SSE2__
          for (int indx = rr * ts + 12 + (FC(rr, 2, filters) & 1), indx1 = indx >> 1; indx < rr * ts + cc1 - 12;
               indx += 8, indx1 += 4)
          {
            vmask copymask = vmaskf_ge(vabsf(zd5v - LVFU(pmwt[indx1])), vabsf(zd5v - LVFU(hvwt[indx1])));

            if (_mm_movemask_ps((vfloat)copymask))
            {   // if for any of the 4 pixels the condition is true, do the maths for all 4 pixels and mask the
              // unused out at the end
              // now interpolate G vertically/horizontally using R+B values
              // unfortunately, since G interpolation cannot be done diagonally this may lead to colour shifts
              // colour ratios for G interpolation
              vfloat rbintv = LVFU(rbint[indx1]);

              // interpolated G via adaptive ratios or Hamilton-Adams in each cardinal direction
              vfloat cruv = vmul2f(LC2VFU(cfa[indx - v1])) / (epsv + rbintv + LVFU(rbint[(indx1 - v1)]));
              vfloat guv  = rbintv * cruv;
              vfloat gu2v = LC2VFU(cfa[indx - v1]) + zd5v * (rbintv - LVFU(rbint[(indx1 - v1)]));
              guv         = vself(vmaskf_lt(vabsf(onev - cruv), arthreshv), guv, gu2v);

              vfloat crdv = vmul2f(LC2VFU(cfa[indx + v1])) / (epsv + rbintv + LVFU(rbint[(indx1 + v1)]));
              vfloat gdv  = rbintv * crdv;
              vfloat gd2v = LC2VFU(cfa[indx + v1]) + zd5v * (rbintv - LVFU(rbint[(indx1 + v1)]));
              gdv         = vself(vmaskf_lt(vabsf(onev - crdv), arthreshv), gdv, gd2v);

              vfloat Gintvv = (LC2VFU(dirwts0[indx - v1]) * gdv + LC2VFU(dirwts0[indx + v1]) * guv)
                              / (LC2VFU(dirwts0[indx + v1]) + LC2VFU(dirwts0[indx - v1]));
              vfloat Gint1v = ULIMV(Gintvv, LC2VFU(cfa[indx - v1]), LC2VFU(cfa[indx + v1]));
              vfloat vwtv   = vmul2f(rbintv - Gintvv) / (epsv + Gintvv + rbintv);
              vfloat Gint2v = vintpf(vwtv, Gintvv, Gint1v);
              Gint1v        = vself(vmaskf_lt(vmul2f(Gintvv), rbintv), Gint1v, Gint2v);
              Gintvv        = vself(vmaskf_lt(Gintvv, rbintv), Gint1v, Gintvv);
              Gintvv        = vself(
                  vmaskf_gt(Gintvv, clip_ptv),
                  ULIMV(Gintvv, LC2VFU(cfa[indx - v1]), LC2VFU(cfa[indx + v1])),
                  Gintvv);

              vfloat crlv = vmul2f(LC2VFU(cfa[indx - 1])) / (epsv + rbintv + LVFU(rbint[(indx1 - 1)]));
              vfloat glv  = rbintv * crlv;
              vfloat gl2v = LC2VFU(cfa[indx - 1]) + zd5v * (rbintv - LVFU(rbint[(indx1 - 1)]));
              glv         = vself(vmaskf_lt(vabsf(onev - crlv), arthreshv), glv, gl2v);

              vfloat crrv = vmul2f(LC2VFU(cfa[indx + 1])) / (epsv + rbintv + LVFU(rbint[(indx1 + 1)]));
              vfloat grv  = rbintv * crrv;
              vfloat gr2v = LC2VFU(cfa[indx + 1]) + zd5v * (rbintv - LVFU(rbint[(indx1 + 1)]));
              grv         = vself(vmaskf_lt(vabsf(onev - crrv), arthreshv), grv, gr2v);

              vfloat Ginthv = (LC2VFU(dirwts1[indx - 1]) * grv + LC2VFU(dirwts1[indx + 1]) * glv)
                              / (LC2VFU(dirwts1[indx - 1]) + LC2VFU(dirwts1[indx + 1]));
              vfloat Gint1h = ULIMV(Ginthv, LC2VFU(cfa[indx - 1]), LC2VFU(cfa[indx + 1]));
              vfloat hwtv   = vmul2f(rbintv - Ginthv) / (epsv + Ginthv + rbintv);
              vfloat Gint2h = vintpf(hwtv, Ginthv, Gint1h);
              Gint1h        = vself(vmaskf_lt(vmul2f(Ginthv), rbintv), Gint1h, Gint2h);
              Ginthv        = vself(vmaskf_lt(Ginthv, rbintv), Gint1h, Ginthv);
              Ginthv        = vself(
                  vmaskf_gt(Ginthv, clip_ptv),
                  ULIMV(Ginthv, LC2VFU(cfa[indx - 1]), LC2VFU(cfa[indx + 1])),
                  Ginthv);

              vfloat greenv = vself(copymask, vintpf(LVFU(hvwt[indx1]), Gintvv, Ginthv), LC2VFU(rgbgreen[indx]));
              STC2VFU(rgbgreen[indx], greenv);

              STVFU(Dgrb[0][indx1], vself(copymask, greenv - LC2VFU(cfa[indx]), LVFU(Dgrb[0][indx1])));
            }
          }

#else

          for (int cc = 12 + (FC(rr, 2, filters) & 1), indx = rr * ts + cc, indx1 = indx >> 1; cc < cc1 - 12;
               cc += 2, indx += 2, indx1++)
          {
            if (fabsf(0.5f - pmwt[indx >> 1]) < fabsf(0.5f - hvwt[indx >> 1]))
            {
              continue;
            }

            // now interpolate G vertically/horizontally using R+B values
            // unfortunately, since G interpolation cannot be done diagonally this may lead to colour shifts

            // colour ratios for G interpolation
            float cru = cfa[indx - v1] * 2.0 / (eps + rbint[indx1] + rbint[(indx1 - v1)]);
            float crd = cfa[indx + v1] * 2.0 / (eps + rbint[indx1] + rbint[(indx1 + v1)]);
            float crl = cfa[indx - 1] * 2.0 / (eps + rbint[indx1] + rbint[(indx1 - 1)]);
            float crr = cfa[indx + 1] * 2.0 / (eps + rbint[indx1] + rbint[(indx1 + 1)]);

            // interpolation of G in four directions
            float gu, gd, gl, gr;

            // interpolated G via adaptive ratios or Hamilton-Adams in each cardinal direction
            if (fabsf(1.f - cru) < arthresh)
            {
              gu = rbint[indx1] * cru;
            }
            else
            {
              gu = cfa[indx - v1] + xdiv2f(rbint[indx1] - rbint[(indx1 - v1)]);
            }

            if (fabsf(1.f - crd) < arthresh)
            {
              gd = rbint[indx1] * crd;
            }
            else
            {
              gd = cfa[indx + v1] + xdiv2f(rbint[indx1] - rbint[(indx1 + v1)]);
            }

            if (fabsf(1.f - crl) < arthresh)
            {
              gl = rbint[indx1] * crl;
            }
            else
            {
              gl = cfa[indx - 1] + xdiv2f(rbint[indx1] - rbint[(indx1 - 1)]);
            }

            if (fabsf(1.f - crr) < arthresh)
            {
              gr = rbint[indx1] * crr;
            }
            else
            {
              gr = cfa[indx + 1] + xdiv2f(rbint[indx1] - rbint[(indx1 + 1)]);
            }

            // interpolated G via adaptive weights of cardinal evaluations
            float Gintv
                = (dirwts0[indx - v1] * gd + dirwts0[indx + v1] * gu) / (dirwts0[indx + v1] + dirwts0[indx - v1]);
            float Ginth = (dirwts1[indx - 1] * gr + dirwts1[indx + 1] * gl) / (dirwts1[indx - 1] + dirwts1[indx + 1]);

            // bound the interpolation in regions of high saturation
            if (Gintv < rbint[indx1])
            {
              if (2 * Gintv < rbint[indx1])
              {
                Gintv = ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]);
              }
              else
              {
                float vwt = 2.0 * (rbint[indx1] - Gintv) / (eps + Gintv + rbint[indx1]);
                Gintv     = vwt * Gintv + (1.f - vwt) * ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]);
              }
            }

            if (Ginth < rbint[indx1])
            {
              if (2 * Ginth < rbint[indx1])
              {
                Ginth = ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]);
              }
              else
              {
                float hwt = 2.0 * (rbint[indx1] - Ginth) / (eps + Ginth + rbint[indx1]);
                Ginth     = hwt * Ginth + (1.f - hwt) * ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]);
              }
            }

            if (Ginth > clip_pt)
            {
              Ginth = ULIM(Ginth, cfa[indx - 1], cfa[indx + 1]);
            }

            if (Gintv > clip_pt)
            {
              Gintv = ULIM(Gintv, cfa[indx - v1], cfa[indx + v1]);
            }

            rgbgreen[indx]     = Ginth * (1.f - hvwt[indx1]) + Gintv * hvwt[indx1];
            Dgrb[0][indx >> 1] = rgbgreen[indx] - cfa[indx];
          }

#endif

        // end of diagonal interpolation correction

        // fancy chrominance interpolation
        //(ey,ex) is location of R site
        for (int rr = 13 - ey; rr < rr1 - 12; rr += 2)
        {
          const int anoying_formated_var = (rr * ts + cc1 - 12) >> 1;
          for (int indx1 = (rr * ts + 13 - ex) >> 1; indx1 < anoying_formated_var; indx1++)
          {                                    // B coset
            Dgrb[1][indx1] = Dgrb[0][indx1];   // split out G-B from G-R
            Dgrb[0][indx1] = 0;
          }
        }
#ifdef __SSE2__
        vfloat oned325v = F2V(1.325f);
        vfloat zd175v   = F2V(0.175f);
        vfloat zd075v   = F2V(0.075f);
#endif

        for (int rr = 14; rr < rr1 - 14; rr++)
#ifdef __SSE2__
          for (int cc = 14 + (FC(rr, 2, filters) & 1), indx = rr * ts + cc, c = 1 - FC(rr, cc, filters) / 2;
               cc < cc1 - 14;
               cc += 8, indx += 8)
          {
            vfloat tempv  = epsv + vabsf(LVFU(Dgrb[c][(indx - m1) >> 1]) - LVFU(Dgrb[c][(indx + m1) >> 1]));
            vfloat temp2v = epsv + vabsf(LVFU(Dgrb[c][(indx + p1) >> 1]) - LVFU(Dgrb[c][(indx - p1) >> 1]));
            vfloat wtnwv  = onev
                           / (tempv + vabsf(LVFU(Dgrb[c][(indx - m1) >> 1]) - LVFU(Dgrb[c][(indx - m3) >> 1]))
                              + vabsf(LVFU(Dgrb[c][(indx + m1) >> 1]) - LVFU(Dgrb[c][(indx - m3) >> 1])));
            vfloat wtnev = onev
                           / (temp2v + vabsf(LVFU(Dgrb[c][(indx + p1) >> 1]) - LVFU(Dgrb[c][(indx + p3) >> 1]))
                              + vabsf(LVFU(Dgrb[c][(indx - p1) >> 1]) - LVFU(Dgrb[c][(indx + p3) >> 1])));
            vfloat wtswv = onev
                           / (temp2v + vabsf(LVFU(Dgrb[c][(indx - p1) >> 1]) - LVFU(Dgrb[c][(indx + m3) >> 1]))
                              + vabsf(LVFU(Dgrb[c][(indx + p1) >> 1]) - LVFU(Dgrb[c][(indx - p3) >> 1])));
            vfloat wtsev = onev
                           / (tempv + vabsf(LVFU(Dgrb[c][(indx + m1) >> 1]) - LVFU(Dgrb[c][(indx - p3) >> 1]))
                              + vabsf(LVFU(Dgrb[c][(indx - m1) >> 1]) - LVFU(Dgrb[c][(indx + m3) >> 1])));

            STVFU(
                Dgrb[c][indx >> 1],
                (wtnwv
                     * (oned325v * LVFU(Dgrb[c][(indx - m1) >> 1]) - zd175v * LVFU(Dgrb[c][(indx - m3) >> 1])
                        - zd075v * (LVFU(Dgrb[c][(indx - m1 - 2) >> 1]) + LVFU(Dgrb[c][(indx - m1 - v2) >> 1])))
                 + wtnev
                       * (oned325v * LVFU(Dgrb[c][(indx + p1) >> 1]) - zd175v * LVFU(Dgrb[c][(indx + p3) >> 1])
                          - zd075v * (LVFU(Dgrb[c][(indx + p1 + 2) >> 1]) + LVFU(Dgrb[c][(indx + p1 + v2) >> 1])))
                 + wtswv
                       * (oned325v * LVFU(Dgrb[c][(indx - p1) >> 1]) - zd175v * LVFU(Dgrb[c][(indx - p3) >> 1])
                          - zd075v * (LVFU(Dgrb[c][(indx - p1 - 2) >> 1]) + LVFU(Dgrb[c][(indx - p1 - v2) >> 1])))
                 + wtsev
                       * (oned325v * LVFU(Dgrb[c][(indx + m1) >> 1]) - zd175v * LVFU(Dgrb[c][(indx + m3) >> 1])
                          - zd075v * (LVFU(Dgrb[c][(indx + m1 + 2) >> 1]) + LVFU(Dgrb[c][(indx + m1 + v2) >> 1]))))
                    / (wtnwv + wtnev + wtswv + wtsev));
          }

#else

          for (int cc = 14 + (FC(rr, 2, filters) & 1), indx = rr * ts + cc, c = 1 - FC(rr, cc, filters) / 2;
               cc < cc1 - 14;
               cc += 2, indx += 2)
          {
            float wtnw = 1.f
                         / (eps + fabsf(Dgrb[c][(indx - m1) >> 1] - Dgrb[c][(indx + m1) >> 1])
                            + fabsf(Dgrb[c][(indx - m1) >> 1] - Dgrb[c][(indx - m3) >> 1])
                            + fabsf(Dgrb[c][(indx + m1) >> 1] - Dgrb[c][(indx - m3) >> 1]));
            float wtne = 1.f
                         / (eps + fabsf(Dgrb[c][(indx + p1) >> 1] - Dgrb[c][(indx - p1) >> 1])
                            + fabsf(Dgrb[c][(indx + p1) >> 1] - Dgrb[c][(indx + p3) >> 1])
                            + fabsf(Dgrb[c][(indx - p1) >> 1] - Dgrb[c][(indx + p3) >> 1]));
            float wtsw = 1.f
                         / (eps + fabsf(Dgrb[c][(indx - p1) >> 1] - Dgrb[c][(indx + p1) >> 1])
                            + fabsf(Dgrb[c][(indx - p1) >> 1] - Dgrb[c][(indx + m3) >> 1])
                            + fabsf(Dgrb[c][(indx + p1) >> 1] - Dgrb[c][(indx - p3) >> 1]));
            float wtse = 1.f
                         / (eps + fabsf(Dgrb[c][(indx + m1) >> 1] - Dgrb[c][(indx - m1) >> 1])
                            + fabsf(Dgrb[c][(indx + m1) >> 1] - Dgrb[c][(indx - p3) >> 1])
                            + fabsf(Dgrb[c][(indx - m1) >> 1] - Dgrb[c][(indx + m3) >> 1]));

            Dgrb[c][indx >> 1]
                = (wtnw
                       * (1.325f * Dgrb[c][(indx - m1) >> 1] - 0.175f * Dgrb[c][(indx - m3) >> 1]
                          - 0.075f * Dgrb[c][(indx - m1 - 2) >> 1] - 0.075f * Dgrb[c][(indx - m1 - v2) >> 1])
                   + wtne
                         * (1.325f * Dgrb[c][(indx + p1) >> 1] - 0.175f * Dgrb[c][(indx + p3) >> 1]
                            - 0.075f * Dgrb[c][(indx + p1 + 2) >> 1] - 0.075f * Dgrb[c][(indx + p1 + v2) >> 1])
                   + wtsw
                         * (1.325f * Dgrb[c][(indx - p1) >> 1] - 0.175f * Dgrb[c][(indx - p3) >> 1]
                            - 0.075f * Dgrb[c][(indx - p1 - 2) >> 1] - 0.075f * Dgrb[c][(indx - p1 - v2) >> 1])
                   + wtse
                         * (1.325f * Dgrb[c][(indx + m1) >> 1] - 0.175f * Dgrb[c][(indx + m3) >> 1]
                            - 0.075f * Dgrb[c][(indx + m1 + 2) >> 1] - 0.075f * Dgrb[c][(indx + m1 + v2) >> 1]))
                  / (wtnw + wtne + wtsw + wtse);
          }

#endif

#ifdef __SSE2__
        int    offset;
        vfloat twov = F2V(2.f);
        vmask  selmask;

        if ((FC(16, 2, filters) & 1) == 1)
        {
          selmask = _mm_set_epi32(0xffffffff, 0, 0xffffffff, 0);
          offset  = 1;
        }
        else
        {
          selmask = _mm_set_epi32(0, 0xffffffff, 0, 0xffffffff);
          offset  = 0;
        }

#endif

        for (int rr = 16; rr < rr1 - 16; rr++)
        {
          int row  = rr + top;
          int col  = left + 16;
          int indx = rr * ts + 16;
#ifdef __SSE2__
          offset  = 1 - offset;
          selmask = vnotm(selmask);

          for (; indx < rr * ts + cc1 - 18 - (cc1 & 1); indx += 4, col += 4)
          {
            if (col < width && row < height)
            {
              vfloat greenv  = LVF(rgbgreen[indx]);
              vfloat temp00v = vdup(LVFU(hvwt[(indx - v1) >> 1]));
              vfloat temp01v = vdup(LVFU(hvwt[(indx + v1) >> 1]));
              vfloat tempv   = onev
                             / (temp00v + twov - vdup(LVFU(hvwt[(indx + 1 + offset) >> 1]))
                                - vdup(LVFU(hvwt[(indx - 1 + offset) >> 1])) + temp01v);

              vfloat redv1 = greenv
                             - (temp00v * vdup(LVFU(Dgrb[0][(indx - v1) >> 1]))
                                + (onev - vdup(LVFU(hvwt[(indx + 1 + offset) >> 1])))
                                      * vdup(LVFU(Dgrb[0][(indx + 1 + offset) >> 1]))
                                + (onev - vdup(LVFU(hvwt[(indx - 1 + offset) >> 1])))
                                      * vdup(LVFU(Dgrb[0][(indx - 1 + offset) >> 1]))
                                + temp01v * vdup(LVFU(Dgrb[0][(indx + v1) >> 1])))
                                   * tempv;
              vfloat bluev1 = greenv
                              - (temp00v * vdup(LVFU(Dgrb[1][(indx - v1) >> 1]))
                                 + (onev - vdup(LVFU(hvwt[(indx + 1 + offset) >> 1])))
                                       * vdup(LVFU(Dgrb[1][(indx + 1 + offset) >> 1]))
                                 + (onev - vdup(LVFU(hvwt[(indx - 1 + offset) >> 1])))
                                       * vdup(LVFU(Dgrb[1][(indx - 1 + offset) >> 1]))
                                 + temp01v * vdup(LVFU(Dgrb[1][(indx + v1) >> 1])))
                                    * tempv;
              vfloat                             redv2  = greenv - vdup(LVFU(Dgrb[0][indx >> 1]));
              vfloat                             bluev2 = greenv - vdup(LVFU(Dgrb[1][indx >> 1]));
              __attribute__((aligned(64))) float _r[4];
              __attribute__((aligned(64))) float _b[4];
              STVF(*_r, vself(selmask, redv1, redv2));
              STVF(*_b, vself(selmask, bluev1, bluev2));
              for (int c = 0; c < 4; c++)
              {
                out[(row * width + col + c) * 4]     = clampnan(_r[c], 0.0, 1.0);
                out[(row * width + col + c) * 4 + 2] = clampnan(_b[c], 0.0, 1.0);
              }
            }
          }

          if (offset == 0)
          {
            for (; indx < rr * ts + cc1 - 16 - (cc1 & 1); indx++, col++)
            {
              if (col < width && row < height)
              {
                float temp = 1.f
                             / (hvwt[(indx - v1) >> 1] + 2.f - hvwt[(indx + 1) >> 1] - hvwt[(indx - 1) >> 1]
                                + hvwt[(indx + v1) >> 1]);
                out[(row * width + col) * 4] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[0][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[0][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[0][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[0][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
                out[(row * width + col) * 4 + 2] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[1][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[1][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[1][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[1][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
              }

              indx++;
              col++;
              if (col < width && row < height)
              {
                out[(row * width + col) * 4]     = clampnan(rgbgreen[indx] - Dgrb[0][indx >> 1], 0.0, 1.0);
                out[(row * width + col) * 4 + 2] = clampnan(rgbgreen[indx] - Dgrb[1][indx >> 1], 0.0, 1.0);
              }
            }

            if (cc1 & 1)
            {   // width of tile is odd
              if (col < width && row < height)
              {
                float temp = 1.f
                             / (hvwt[(indx - v1) >> 1] + 2.f - hvwt[(indx + 1) >> 1] - hvwt[(indx - 1) >> 1]
                                + hvwt[(indx + v1) >> 1]);
                out[(row * width + col) * 4] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[0][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[0][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[0][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[0][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
                out[(row * width + col) * 4 + 2] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[1][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[1][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[1][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[1][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
              }
            }
          }
          else
          {
            for (; indx < rr * ts + cc1 - 16 - (cc1 & 1); indx++, col++)
            {
              if (col < width && row < height)
              {
                out[(row * width + col) * 4]     = clampnan(rgbgreen[indx] - Dgrb[0][indx >> 1], 0.0, 1.0);
                out[(row * width + col) * 4 + 2] = clampnan(rgbgreen[indx] - Dgrb[1][indx >> 1], 0.0, 1.0);
              }

              indx++;
              col++;
              if (col < width && row < height)
              {
                float temp = 1.f
                             / (hvwt[(indx - v1) >> 1] + 2.f - hvwt[(indx + 1) >> 1] - hvwt[(indx - 1) >> 1]
                                + hvwt[(indx + v1) >> 1]);
                out[(row * width + col) * 4] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[0][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[0][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[0][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[0][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
                out[(row * width + col) * 4 + 2] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[1][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[1][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[1][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[1][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
              }
            }

            if (cc1 & 1)
            {   // width of tile is odd
              if (col < width && row < height)
              {
                out[(row * width + col) * 4]     = clampnan(rgbgreen[indx] - Dgrb[0][indx >> 1], 0.0, 1.0);
                out[(row * width + col) * 4 + 2] = clampnan(rgbgreen[indx] - Dgrb[1][indx >> 1], 0.0, 1.0);
              }
            }
          }

#else

          if ((FC(rr, 2, filters) & 1) == 1)
          {
            for (; indx < rr * ts + cc1 - 16 - (cc1 & 1); indx++, col++)
            {
              if (col < width && row < height)
              {
                float temp = 1.f
                             / (hvwt[(indx - v1) >> 1] + 2.f - hvwt[(indx + 1) >> 1] - hvwt[(indx - 1) >> 1]
                                + hvwt[(indx + v1) >> 1]);
                out[(row * width + col) * 4] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[0][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[0][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[0][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[0][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
                out[(row * width + col) * 4 + 2] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[1][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[1][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[1][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[1][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
              }

              indx++;
              col++;
              if (col < width && row < height)
              {
                out[(row * width + col) * 4]     = clampnan(rgbgreen[indx] - Dgrb[0][indx >> 1], 0.0, 1.0);
                out[(row * width + col) * 4 + 2] = clampnan(rgbgreen[indx] - Dgrb[1][indx >> 1], 0.0, 1.0);
              }
            }

            if (cc1 & 1)
            {   // width of tile is odd
              if (col < width && row < height)
              {
                float temp = 1.f
                             / (hvwt[(indx - v1) >> 1] + 2.f - hvwt[(indx + 1) >> 1] - hvwt[(indx - 1) >> 1]
                                + hvwt[(indx + v1) >> 1]);
                out[(row * width + col) * 4] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[0][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[0][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[0][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[0][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
                out[(row * width + col) * 4 + 2] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[1][(indx - v1) >> 1]
                           + (1.f - hvwt[(indx + 1) >> 1]) * Dgrb[1][(indx + 1) >> 1]
                           + (1.f - hvwt[(indx - 1) >> 1]) * Dgrb[1][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[1][(indx + v1) >> 1])
                              * temp,
                    0.0,
                    1.0);
              }
            }
          }
          else
          {
            for (; indx < rr * ts + cc1 - 16 - (cc1 & 1); indx++, col++)
            {
              if (col < width && row < height)
              {
                out[(row * width + col) * 4]     = clampnan(rgbgreen[indx] - Dgrb[0][indx >> 1], 0.0f, 1.0f);
                out[(row * width + col) * 4 + 2] = clampnan(rgbgreen[indx] - Dgrb[1][indx >> 1], 0.0f, 1.0f);
              }

              indx++;
              col++;
              if (col < width && row < height)
              {
                float temp = 1.f
                             / (hvwt[(indx - v1) >> 1] + 2.f - hvwt[(indx + 1) >> 1] - hvwt[(indx - 1) >> 1]
                                + hvwt[(indx + v1) >> 1]);

                out[(row * width + col) * 4] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[0][(indx - v1) >> 1]
                           + (1.0f - hvwt[(indx + 1) >> 1]) * Dgrb[0][(indx + 1) >> 1]
                           + (1.0f - hvwt[(indx - 1) >> 1]) * Dgrb[0][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[0][(indx + v1) >> 1])
                              * temp,
                    0.0f,
                    1.0f);

                out[(row * width + col) * 4 + 2] = clampnan(
                    rgbgreen[indx]
                        - ((hvwt[(indx - v1) >> 1]) * Dgrb[1][(indx - v1) >> 1]
                           + (1.0f - hvwt[(indx + 1) >> 1]) * Dgrb[1][(indx + 1) >> 1]
                           + (1.0f - hvwt[(indx - 1) >> 1]) * Dgrb[1][(indx - 1) >> 1]
                           + (hvwt[(indx + v1) >> 1]) * Dgrb[1][(indx + v1) >> 1])
                              * temp,
                    0.0f,
                    1.0f);
              }
            }

            if (cc1 & 1)
            {   // width of tile is odd
              if (col < width && row < height)
              {
                out[(row * width + col) * 4]     = clampnan(rgbgreen[indx] - Dgrb[0][indx >> 1], 0.0f, 1.0f);
                out[(row * width + col) * 4 + 2] = clampnan(rgbgreen[indx] - Dgrb[1][indx >> 1], 0.0f, 1.0f);
              }
            }
          }

#endif
        }

        // copy smoothed results back to image matrix
        for (int rr = 16; rr < rr1 - 16; rr++)
        {
          int row = rr + top;
          int cc  = 16;
          // TODO (darktable): we have the pixel colors interleaved so writing them in blocks using SSE2 is
          // not possible. or is it?
          // #ifdef __SSE2__
          //
          //           for(; cc < cc1 - 19; cc += 4)
          //           {
          //             STVFU(out[(row *   width + (cc + left))  * 4 + 1], LVF(rgbgreen[rr * ts +
          //             cc]));
          //           }
          //
          // #endif

          for (; cc < cc1 - 16; cc++)
          {
            int col  = cc + left;
            int indx = rr * ts + cc;
            if (col < width && row < height) out[(row * width + col) * 4 + 1] = clampnan(rgbgreen[indx], 0.0f, 1.0f);
          }
        }
      }
    }   // end of main loop

    // clean up
    free(buffer);
  }
}


void border_interpolate(
    int          winw,
    int          winh,
    int          lborders,
    const float *rawData,
    float *      red,
    float *      green,
    float *      blue,
    unsigned int filters)
{
  int bord   = lborders;
  int width  = winw;
  int height = winh;

  for (int i = 0; i < height; i++)
  {
    float sum[6];

    for (int j = 0; j < bord; j++)
    {
      // first few columns
      memset(sum, 0, 6 * sizeof(float));

      for (int i1 = i - 1; i1 < i + 2; i1++)
        for (int j1 = j - 1; j1 < j + 2; j1++)
        {
          if ((i1 > -1) && (i1 < height) && (j1 > -1))
          {
            int c = FC(i1, j1, filters);
            sum[c] += rawData[i1 * width + j1];
            sum[c + 3]++;
          }
        }

      int c = FC(i, j, filters);

      if (c == 1)
      {
        red[i * width + j]   = sum[0] / sum[3];
        green[i * width + j] = rawData[i * width + j];
        blue[i * width + j]  = sum[2] / sum[5];
      }
      else
      {
        green[i * width + j] = sum[1] / sum[4];

        if (c == 0)
        {
          red[i * width + j]  = rawData[i * width + j];
          blue[i * width + j] = sum[2] / sum[5];
        }
        else
        {
          red[i * width + j]  = sum[0] / sum[3];
          blue[i * width + j] = rawData[i * width + j];
        }
      }
    }   //j

    for (int j = width - bord; j < width; j++)
    {   //last few columns
      for (int c = 0; c < 6; c++)
      {
        sum[c] = 0;
      }

      for (int i1 = i - 1; i1 < i + 2; i1++)
        for (int j1 = j - 1; j1 < j + 2; j1++)
        {
          if ((i1 > -1) && (i1 < height) && (j1 < width))
          {
            int c = FC(i1, j1, filters);
            sum[c] += rawData[i1 * width + j1];
            sum[c + 3]++;
          }
        }

      int c = FC(i, j, filters);

      if (c == 1)
      {
        red[i * width + j]   = sum[0] / sum[3];
        green[i * width + j] = rawData[i * width + j];
        blue[i * width + j]  = sum[2] / sum[5];
      }
      else
      {
        green[i * width + j] = sum[1] / sum[4];

        if (c == 0)
        {
          red[i * width + j]  = rawData[i * width + j];
          blue[i * width + j] = sum[2] / sum[5];
        }
        else
        {
          red[i * width + j]  = sum[0] / sum[3];
          blue[i * width + j] = rawData[i * width + j];
        }
      }
    }   //j
  }     //i

  for (int i = 0; i < bord; i++)
  {
    float sum[6];

    for (int j = bord; j < width - bord; j++)
    {   //first few rows
      for (int c = 0; c < 6; c++)
      {
        sum[c] = 0;
      }

      for (int i1 = i - 1; i1 < i + 2; i1++)
        for (int j1 = j - 1; j1 < j + 2; j1++)
        {
          if ((i1 > -1) && (i1 < height) && (j1 > -1))
          {
            int c = FC(i1, j1, filters);
            sum[c] += rawData[i1 * width + j1];
            sum[c + 3]++;
          }
        }

      int c = FC(i, j, filters);

      if (c == 1)
      {
        red[i * width + j]   = sum[0] / sum[3];
        green[i * width + j] = rawData[i * width + j];
        blue[i * width + j]  = sum[2] / sum[5];
      }
      else
      {
        green[i * width + j] = sum[1] / sum[4];

        if (c == 0)
        {
          red[i * width + j]  = rawData[i * width + j];
          blue[i * width + j] = sum[2] / sum[5];
        }
        else
        {
          red[i * width + j]  = sum[0] / sum[3];
          blue[i * width + j] = rawData[i * width + j];
        }
      }
    }   //j
  }

  for (int i = height - bord; i < height; i++)
  {
    float sum[6];

    for (int j = bord; j < width - bord; j++)
    {   //last few rows
      for (int c = 0; c < 6; c++)
      {
        sum[c] = 0;
      }

      for (int i1 = i - 1; i1 < i + 2; i1++)
        for (int j1 = j - 1; j1 < j + 2; j1++)
        {
          if ((i1 > -1) && (i1 < height) && (j1 < width))
          {
            int c = FC(i1, j1, filters);
            sum[c] += rawData[i1 * width + j1];
            sum[c + 3]++;
          }
        }

      int c = FC(i, j, filters);

      if (c == 1)
      {
        red[i * width + j]   = sum[0] / sum[3];
        green[i * width + j] = rawData[i * width + j];
        blue[i * width + j]  = sum[2] / sum[5];
      }
      else
      {
        green[i * width + j] = sum[1] / sum[4];

        if (c == 0)
        {
          red[i * width + j]  = rawData[i * width + j];
          blue[i * width + j] = sum[2] / sum[5];
        }
        else
        {
          red[i * width + j]  = sum[0] / sum[3];
          blue[i * width + j] = rawData[i * width + j];
        }
      }
    }   //j
  }
}

bool ISRED(unsigned row, unsigned col, unsigned filters)
{
  return ((filters >> ((((row) << 1 & 14) + ((col)&1)) << 1) & 3) == 0);
}
bool ISGREEN(unsigned row, unsigned col, unsigned filters)
{
  return ((filters >> ((((row) << 1 & 14) + ((col)&1)) << 1) & 3) == 1);
}
bool ISBLUE(unsigned row, unsigned col, unsigned filters)
{
  return ((filters >> ((((row) << 1 & 14) + ((col)&1)) << 1) & 3) == 2);
}

inline void vng4interpolate_row_redblue(
    const float *      rawData,
    float *            ar,
    float *            ab,
    const float *const pg,
    const float *const cg,
    const float *const ng,
    int                i,
    int                width,
    unsigned int       filters)
{
  if (ISBLUE(i, 0, filters) || ISBLUE(i, 1, filters))
  {
    std::swap(ar, ab);
  }

  // RGRGR or GRGRGR line
  for (int j = 3; j < width - 3; ++j)
  {
    if (!ISGREEN(i, j, filters))
    {
      // keep original value
      ar[j] = rawData[i * width + j];
      // cross interpolation of red/blue
      float rb = (rawData[(i - 1) * width + j - 1] - pg[j - 1] + rawData[(i + 1) * width + j - 1] - ng[j - 1]);
      rb += (rawData[(i - 1) * width + j + 1] - pg[j + 1] + rawData[(i + 1) * width + j + 1] - ng[j + 1]);
      ab[j] = std::max(0.f, cg[j] + rb * 0.25f);
    }
    else
    {
      // linear R/B-G interpolation horizontally
      ar[j] = std::max(
          0.f,
          cg[j] + (rawData[i * width + j - 1] - cg[j - 1] + rawData[i * width + j + 1] - cg[j + 1]) / 2);
      // linear B/R-G interpolation vertically
      ab[j] = std::max(0.f, cg[j] + (rawData[(i - 1) * width + j] - pg[j] + rawData[(i + 1) * width + j] - ng[j]) / 2);
    }
  }
}


extern "C" void
vng4_demosaic_rgb(const float *rawData, float *red, float *green, float *blue, size_t w, size_t h, unsigned int filters)
{
  memset(red, 0, w * h * sizeof(float));
  memset(green, 0, w * h * sizeof(float));
  memset(blue, 0, w * h * sizeof(float));
  // // Test for RGB cfa
  // for (int i = 0; i < 2; i++) {
  //     for (int j = 0; j < 2; j++) {
  //         if (FC(i, j) == 3) {
  //             // avoid crash
  //             std::cout << "vng4_demosaic supports only RGB Colour filter arrays. Falling back to igv_interpolate" << std::endl;
  //             // igv_interpolate(W, H);
  //             return;
  //         }
  //     }
  // }

  const signed short int *cp,
      terms[] = {-2, -2, +0, -1, 0, 0x01, -2, -2, +0, +0, 1, 0x01, -2, -1, -1, +0, 0, 0x01, -2, -1, +0, -1, 0, 0x02,
                 -2, -1, +0, +0, 0, 0x03, -2, -1, +0, +1, 1, 0x01, -2, +0, +0, -1, 0, 0x06, -2, +0, +0, +0, 1, 0x02,
                 -2, +0, +0, +1, 0, 0x03, -2, +1, -1, +0, 0, 0x04, -2, +1, +0, -1, 1, 0x04, -2, +1, +0, +0, 0, 0x06,
                 -2, +1, +0, +1, 0, 0x02, -2, +2, +0, +0, 1, 0x04, -2, +2, +0, +1, 0, 0x04, -1, -2, -1, +0, 0, 0x80,
                 -1, -2, +0, -1, 0, 0x01, -1, -2, +1, -1, 0, 0x01, -1, -2, +1, +0, 1, 0x01, -1, -1, -1, +1, 0, 0x88,
                 -1, -1, +1, -2, 0, 0x40, -1, -1, +1, -1, 0, 0x22, -1, -1, +1, +0, 0, 0x33, -1, -1, +1, +1, 1, 0x11,
                 -1, +0, -1, +2, 0, 0x08, -1, +0, +0, -1, 0, 0x44, -1, +0, +0, +1, 0, 0x11, -1, +0, +1, -2, 1, 0x40,
                 -1, +0, +1, -1, 0, 0x66, -1, +0, +1, +0, 1, 0x22, -1, +0, +1, +1, 0, 0x33, -1, +0, +1, +2, 1, 0x10,
                 -1, +1, +1, -1, 1, 0x44, -1, +1, +1, +0, 0, 0x66, -1, +1, +1, +1, 0, 0x22, -1, +1, +1, +2, 0, 0x10,
                 -1, +2, +0, +1, 0, 0x04, -1, +2, +1, +0, 1, 0x04, -1, +2, +1, +1, 0, 0x04, +0, -2, +0, +0, 1, 0x80,
                 +0, -1, +0, +1, 1, 0x88, +0, -1, +1, -2, 0, 0x40, +0, -1, +1, +0, 0, 0x11, +0, -1, +2, -2, 0, 0x40,
                 +0, -1, +2, -1, 0, 0x20, +0, -1, +2, +0, 0, 0x30, +0, -1, +2, +1, 1, 0x10, +0, +0, +0, +2, 1, 0x08,
                 +0, +0, +2, -2, 1, 0x40, +0, +0, +2, -1, 0, 0x60, +0, +0, +2, +0, 1, 0x20, +0, +0, +2, +1, 0, 0x30,
                 +0, +0, +2, +2, 1, 0x10, +0, +1, +1, +0, 0, 0x44, +0, +1, +1, +2, 0, 0x10, +0, +1, +2, -1, 1, 0x40,
                 +0, +1, +2, +0, 0, 0x60, +0, +1, +2, +1, 0, 0x20, +0, +1, +2, +2, 0, 0x10, +1, -2, +1, +0, 0, 0x80,
                 +1, -1, +1, +1, 0, 0x88, +1, +0, +1, +2, 0, 0x08, +1, +0, +2, -1, 0, 0x40, +1, +0, +2, +1, 0, 0x10},
      chood[] = {-1, -1, -1, 0, -1, +1, 0, +1, +1, +1, +1, 0, +1, -1, 0, -1};

  // double progress = 0.0;
  // const bool plistenerActive = plistener;

  // if (plistenerActive) {
  //     plistener->setProgressStr (Glib::ustring::compose(M("TP_RAW_DMETHOD_PROGRESSBAR"), M("TP_RAW_VNG4")));
  //     plistener->setProgress (progress);
  // }

  unsigned prefilters = filters;

  // separate out G1 and G2 in RGGB Bayer patterns
  // Thanks Darktable ;-)
  if ((filters & 3) == 1)
    prefilters = filters | 0x03030303u;
  else
    prefilters = filters | 0x0c0c0c0cu;

  const int              width = w, height = h;
  constexpr unsigned int colors = 4;

  float(*image)[4] = (float(*)[4])calloc(static_cast<size_t>(height) * width, sizeof *image);

  int   lcode[16][16][32];
  float mul[16][16][8];
  float csum[16][16][3];

  // first linear interpolation
  for (int row = 0; row < 16; row++)
    for (int col = 0; col < 16; col++)
    {
      int * ip       = lcode[row][col];
      int   mulcount = 0;
      float sum[4]   = {};

      for (int y = -1; y <= 1; y++)
        for (int x = -1; x <= 1; x++)
        {
          int shift = (y == 0) + (x == 0);

          if (shift == 2)
          {
            continue;
          }

          int color = FC(row + y, col + x, prefilters);
          *ip++     = (width * y + x) * 4 + color;

          mul[row][col][mulcount] = (1 << shift);
          *ip++                   = color;
          sum[color] += (1 << shift);
          mulcount++;
        }

      int colcount = 0;

      for (unsigned int c = 0; c < colors; c++)
        if (c != FC(row, col, prefilters))
        {
          *ip++                    = c;
          csum[row][col][colcount] = 1.f / sum[c];
          colcount++;
        }
    }

#ifdef _OPENMP
    #pragma omp parallel
#endif
  {
    int firstRow = -1;
    int lastRow  = -1;
#ifdef _OPENMP
    // note, static scheduling is important in this implementation
    #pragma omp for schedule(static)
#endif

    for (int ii = 0; ii < height; ii++)
    {
      if (firstRow == -1)
      {
        firstRow = ii;
      }
      lastRow = ii;
      for (int jj = 0; jj < width; jj++)
      {
        image[ii * width + jj][FC(ii, jj, prefilters)] = rawData[ii * width + jj];
      }
      if (ii - 1 > firstRow)
      {
        int row = ii - 1;
        for (int col = 1; col < width - 1; col++)
        {
          float *pix    = image[row * width + col];
          int *  ip     = lcode[row & 15][col & 15];
          float  sum[4] = {};

          for (int i = 0; i < 8; i++, ip += 2)
          {
            sum[ip[1]] += pix[ip[0]] * mul[row & 15][col & 15][i];
          }

          for (unsigned int i = 0; i < colors - 1; i++, ip++)
          {
            pix[ip[0]] = sum[ip[0]] * csum[row & 15][col & 15][i];
          }
        }
      }
    }

    // now all rows are processed except the first and last row of each chunk
    // let's process them now but skip row 0 and row H - 1
    if (firstRow > 0 && firstRow < height - 1)
    {
      const int row = firstRow;
      for (int col = 1; col < width - 1; col++)
      {
        float *pix    = image[row * width + col];
        int *  ip     = lcode[row & 15][col & 15];
        float  sum[4] = {};

        for (int i = 0; i < 8; i++, ip += 2)
        {
          sum[ip[1]] += pix[ip[0]] * mul[row & 15][col & 15][i];
        }

        for (unsigned int i = 0; i < colors - 1; i++, ip++)
        {
          pix[ip[0]] = sum[ip[0]] * csum[row & 15][col & 15][i];
        }
      }
    }

    if (lastRow > 0 && lastRow < height - 1)
    {
      const int row = lastRow;
      for (int col = 1; col < width - 1; col++)
      {
        float *pix    = image[row * width + col];
        int *  ip     = lcode[row & 15][col & 15];
        float  sum[4] = {};

        for (int i = 0; i < 8; i++, ip += 2)
        {
          sum[ip[1]] += pix[ip[0]] * mul[row & 15][col & 15][i];
        }

        for (unsigned int i = 0; i < colors - 1; i++, ip++)
        {
          pix[ip[0]] = sum[ip[0]] * csum[row & 15][col & 15][i];
        }
      }
    }
  }

  constexpr int prow = 7, pcol = 1;
  int32_t *     code[8][2];
  int32_t *     ip = (int32_t *)calloc((prow + 1) * (pcol + 1), 1280);

  for (int row = 0; row <= prow; row++) /* Precalculate for VNG */
    for (int col = 0; col <= pcol; col++)
    {
      code[row][col] = ip;
      cp             = terms;
      for (int t = 0; t < 64; t++)
      {
        int          y1     = *cp++;
        int          x1     = *cp++;
        int          y2     = *cp++;
        int          x2     = *cp++;
        int          weight = *cp++;
        int          grads  = *cp++;
        unsigned int color  = FC(row + y1, col + x1, prefilters);

        if (FC(row + y2, col + x2, prefilters) != color)
        {
          continue;
        }

        int diag = (FC(row, col + 1, prefilters) == color && FC(row + 1, col, prefilters) == color) ? 2 : 1;

        if (abs(y1 - y2) == diag && abs(x1 - x2) == diag)
        {
          continue;
        }

        *ip++ = (y1 * width + x1) * 4 + color;
        *ip++ = (y2 * width + x2) * 4 + color;
#ifdef __SSE2__
        // at least on machines with SSE2 feature this cast is save
        *reinterpret_cast<float *>(ip++) = 1 << weight;
#else
        *ip++ = 1 << weight;
#endif
        for (int g = 0; g < 8; g++)
          if (grads & (1 << g))
          {
            *ip++ = g;
          }

        *ip++ = -1;
      }

      *ip++ = INT_MAX;

      cp = chood;
      for (int g = 0; g < 8; g++)
      {
        int y              = *cp++;
        int x              = *cp++;
        *ip++              = (y * width + x) * 4;
        unsigned int color = FC(row, col, prefilters);

        if (FC(row + y, col + x, prefilters) != color && FC(row + y * 2, col + x * 2, prefilters) == color)
        {
          *ip++ = (y * width + x) * 8 + color;
        }
        else
        {
          *ip++ = 0;
        }
      }
    }

    // if(plistenerActive) {
    //     progress = 0.2;
    //     plistener->setProgress (progress);
    // }

#ifdef _OPENMP
    #pragma omp parallel
#endif
  {
    // constexpr int progressStep = 64;
    // const double progressInc = (1.0 - progress) / ((height - 2) / progressStep);
    int firstRow = -1;
    int lastRow  = -1;
#ifdef _OPENMP
    // note, static scheduling is important in this implementation
    #pragma omp for schedule(static)
#endif

    for (int row = 2; row < height - 2; row++)
    { /* Do VNG interpolation */
      if (firstRow == -1)
      {
        firstRow = row;
      }
      lastRow = row;
      for (int col = 2; col < width - 2; col++)
      {
        float *  pix     = image[row * width + col];
        int      color   = FC(row, col, prefilters);
        int32_t *ip      = code[row & prow][col & pcol];
        float    gval[8] = {};

        while (ip[0] != INT_MAX)
        { /* Calculate gradients */
#ifdef __SSE2__
          // at least on machines with SSE2 feature this cast is save and saves a lot of int => float conversions
          const float diff = std::fabs(pix[ip[0]] - pix[ip[1]]) * reinterpret_cast<float *>(ip)[2];
#else
          const float diff = std::fabs(pix[ip[0]] - pix[ip[1]]) * ip[2];
#endif
          gval[ip[3]] += diff;
          ip += 5;
          if (UNLIKELY(ip[-1] != -1))
          {
            gval[ip[-1]] += diff;
            ip++;
          }
        }
        ip++;

        float gMin = gval[0];
        float gMax = gval[0];

        for (int g = 1; g < 8; g++)
        {
          gMin = std::min(gMin, gval[g]);
          gMax = std::max(gMax, gval[g]);
        }

        const float thold = 1.5 * gMin + 0.5f * (gMax + gMin);

        float       sum0     = 0.f;
        float       sum1     = 0.f;
        const float greenval = pix[color];
        int         num      = 0;

        if (color & 1)
        {
          color ^= 2;
          for (int g = 0; g < 8; g++, ip += 2)
          { /* Average the neighbors */
            if (gval[g] <= thold)
            {
              if (ip[1])
              {
                sum0 += greenval + pix[ip[1]];
              }

              sum1 += pix[ip[0] + color];
              num++;
            }
          }
          sum0 *= 0.5f;
        }
        else
        {
          for (int g = 0; g < 8; g++, ip += 2)
          { /* Average the neighbors */
            if (gval[g] <= thold)
            {
              if (ip[1])
              {
                sum0 += greenval + pix[ip[1]];
              }

              sum1 += pix[ip[0] + 1] + pix[ip[0] + 3];
              num++;
            }
          }
        }
        //sum1 = 0;
        //  std::cout << sum1 << " " << sum0 << " " << greenval << std::endl;
        green[row * w + col] = std::max(0.f, greenval + (sum1 - sum0) / (2 * num));
      }
      if (row - 1 > firstRow)
      {
        vng4interpolate_row_redblue(
            rawData,
            &red[(row - 1) * width],
            &blue[(row - 1) * width],
            &green[(row - 2) * width],
            &green[(row - 1) * width],
            &green[row * width],
            row - 1,
            w,
            filters);
      }

      //             if(plistenerActive) {
      //                 if((row % progressStep) == 0)
      // #ifdef _OPENMP
      //                     #pragma omp critical (updateprogress)
      // #endif
      //                 {
      //                     progress += progressInc;
      //                     plistener->setProgress (progress);
      //                 }
      //             }
    }

    if (firstRow > 2 && firstRow < height - 3)
    {
      vng4interpolate_row_redblue(
          rawData,
          &red[firstRow * width],
          &blue[firstRow * width],
          &green[(firstRow - 1) * width],
          &green[firstRow * width],
          &green[(firstRow + 1) * width],
          firstRow,
          w,
          filters);
    }

    if (lastRow > 2 && lastRow < height - 3)
    {
      vng4interpolate_row_redblue(
          rawData,
          &red[lastRow * width],
          &blue[lastRow * width],
          &green[(lastRow - 1) * width],
          &green[lastRow * width],
          &green[(lastRow + 1) * width],
          lastRow,
          w,
          filters);
    }
#ifdef _OPENMP
    #pragma omp single
#endif
    {
      // let the first thread, which is out of work, do the border interpolation
      border_interpolate(w, h, 3, rawData, red, green, blue, filters);
    }
  }

  free(code[0][0]);
  free(image);

  // if(plistenerActive) {
  //     plistener->setProgress (1.0);
  // }
}


/*==================================================================================
 * end of raw therapee code
 *==================================================================================*/

extern "C" void
vgn4_demosaic(const float *rawData, float *debayered_image, size_t width, size_t height, unsigned int filters)
{
  float *debayered_r_temp = new float[width * height];
  float *debayered_g_temp = new float[width * height];
  float *debayered_b_temp = new float[width * height];

  vng4_demosaic_rgb(rawData, debayered_r_temp, debayered_g_temp, debayered_b_temp, width, height, filters);

  for (size_t row = 0; row < height; row++)
  {
    for (size_t col = 0; col < width; col++)
    {
      debayered_image[3 * (row * width + col) + 0] = debayered_r_temp[row * width + col];
      debayered_image[3 * (row * width + col) + 1] = debayered_g_temp[row * width + col];
      debayered_image[3 * (row * width + col) + 2] = debayered_b_temp[row * width + col];
    }
  }

  delete[] debayered_r_temp;
  delete[] debayered_g_temp;
  delete[] debayered_b_temp;
}


// Rawtherapee code
constexpr int MAXVAL = 0xffff;

// template<typename T>
// constexpr const T &LIM(const T &val, const T &low, const T &high)
// {
//   return max(low, min(val, high));
// }

template<typename T>
constexpr T CLIP(const T &a)
{
  return LIM(a, static_cast<T>(0), static_cast<T>(MAXVAL));
}

template<typename T>
inline T median(T a, T b, T c)
{
  return std::max(std::min(a, b), std::min(c, std::max(a, b)));
}

// #ifdef __SSE2__
// template<>
// inline vfloat median(float a, float b, float c)
// {
//     return vmaxf(vminf(a, b), vminf(c, vmaxf(a, b)));
// }
// #else
// template<typename T>
// inline T median(std::array<T, 3> array)
// {
//     return std::max(std::min(a, b), std::min(c, std::max(a, b)));
// }
// #endif



unsigned fc(const unsigned int cfa[2][2], int r, int c)
{
  return cfa[r & 1][c & 1];
}

#define TS 144

extern "C" void
ahd_demosaic_rgb(const float *rawData, float *red, float *green, float *blue, size_t w, size_t h, unsigned int filters)
{
  const unsigned int cfa[2][2] = {{FC(0, 0, filters), FC(0, 1, filters)}, {FC(1, 0, filters), FC(1, 1, filters)}};
  constexpr int      dirs[4]   = {-1, 1, -TS, TS};
  float              xyz_cam[3][3];
  float *            cbrt = new float[65536];

  int width = w, height = h;

  // clang-format off
  constexpr float xyz_rgb[3][3] = {        /* XYZ from RGB */
    { 0.412453f, 0.357580f, 0.180423f },
    { 0.212671f, 0.715160f, 0.072169f },
    { 0.019334f, 0.119193f, 0.950227f }
  };
  // clang-format on

  constexpr float d65_white[3] = {0.950456f, 1.f, 1.088754f};

  double progress = 0.0;

  // if (plistener) {
  //     plistener->setProgressStr (Glib::ustring::compose(M("TP_RAW_DMETHOD_PROGRESSBAR"), M("TP_RAW_AHD")));
  //     plistener->setProgress (progress);
  // }

  for (int i = 0; i < 65536; i++)
  {
    const double r = i / 65535.0;
    cbrt[i]        = r > 0.008856 ? std::cbrt(r) : 7.787 * r + 16 / 116.0;
  }

  for (int i = 0; i < 3; i++)
  {
    for (unsigned int j = 0; j < 3; j++)
    {
      xyz_cam[i][j] = 0;
      for (int k = 0; k < 3; k++)
      {
        // TODO
        xyz_cam[i][j] += xyz_rgb[i][k] /* * static_cast<float>(imatrices.rgb_cam[k][j])*/ / d65_white[i];
      }
    }
  }
  border_interpolate(w, h, 5, rawData, red, green, blue, filters);


#ifdef _OPENMP
#pragma omp parallel
#endif
  {
    int    progresscounter = 0;
    float *buffer          = new float[13 * TS * TS]; /* 1053 kB per core */
    auto   rgb             = (float(*)[TS][TS][3])buffer;
    auto   lab             = (float(*)[TS][TS][3])(buffer + 6 * TS * TS);
    auto   homo            = (uint16_t(*)[TS][TS])(buffer + 12 * TS * TS);

#ifdef _OPENMP
    #pragma omp for collapse(2) schedule(dynamic) nowait
#endif
    for (int top = 2; top < height - 5; top += TS - 6)
    {
      for (int left = 2; left < width - 5; left += TS - 6)
      {
        //  Interpolate green horizontally and vertically:
        for (int row = top; row < top + TS && row < height - 2; row++)
        {
          for (int col = left + (fc(cfa, row, left) & 1); col < std::min(left + TS, width - 2); col += 2)
          {
            auto  pix  = &rawData[row * width + col];
            float val0 = 0.25f * ((pix[-1] + pix[0] + pix[1]) * 2 - pix[-2] - pix[2]);

            rgb[0][row - top][col - left][1] = median(val0, pix[-1], pix[1]);

            float val1 = 0.25f * ((pix[-width] + pix[0] + pix[width]) * 2 - pix[-2 * width] - pix[2 * width]);

            rgb[1][row - top][col - left][1] = median(val1, pix[-width], pix[width]);
          }
        }

        //  Interpolate red and blue, and convert to CIELab:
        for (int d = 0; d < 2; d++)
          for (int row = top + 1; row < top + TS - 1 && row < height - 3; row++)
          {
            int cng = fc(cfa, row + 1, fc(cfa, row + 1, 0) & 1);
            for (int col = left + 1; col < std::min(left + TS - 1, width - 3); col++)
            {
              auto pix = &rawData[row * width + col];
              auto rix = &rgb[d][row - top][col - left];
              auto lix = lab[d][row - top][col - left];
              if (fc(cfa, row, col) == 1)
              {
                rix[0][2 - cng] = CLIP(pix[0] + (0.5f * (pix[-1] + pix[1] - rix[-1][1] - rix[1][1])));
                rix[0][cng]     = CLIP(pix[0] + (0.5f * (pix[-width] + pix[width] - rix[-TS][1] - rix[TS][1])));
                rix[0][1]       = pix[0];
              }
              else
              {
                rix[0][cng] = CLIP(
                    rix[0][1]
                    + (0.25f
                       * (pix[-width - 1] + pix[-width + 1] + pix[+width - 1] + pix[+width + 1] - rix[-TS - 1][1]
                          - rix[-TS + 1][1] - rix[+TS - 1][1] - rix[+TS + 1][1])));
                rix[0][2 - cng] = pix[0];
              }
              float xyz[3] = {};

              for (unsigned int c = 0; c < 3; ++c)
              {
                xyz[0] += xyz_cam[0][c] * rix[0][c];
                xyz[1] += xyz_cam[1][c] * rix[0][c];
                xyz[2] += xyz_cam[2][c] * rix[0][c];
              }
              // Check...
              for (unsigned int c = 0; c < 3; ++c)
              {
                xyz[c] = cbrt[std::max(0, std::min(int(65535.f * xyz[c]), 65535))];
              }

              lix[0] = 116.f * xyz[1] - 16.f;
              lix[1] = 500.f * (xyz[0] - xyz[1]);
              lix[2] = 200.f * (xyz[1] - xyz[2]);
            }
          }

        //  Build homogeneity maps from the CIELab images:

        for (int row = top + 2; row < top + TS - 2 && row < height - 4; row++)
        {
          int   tr = row - top;
          float ldiff[2][4], abdiff[2][4];

          for (int col = left + 2, tc = 2; col < left + TS - 2 && col < width - 4; col++, tc++)
          {
            for (int d = 0; d < 2; d++)
            {
              auto lix = &lab[d][tr][tc];

              for (int i = 0; i < 4; i++)
              {
                ldiff[d][i]  = std::fabs(lix[0][0] - lix[dirs[i]][0]);
                abdiff[d][i] = SQR(lix[0][1] - lix[dirs[i]][1]) + SQR(lix[0][2] - lix[dirs[i]][2]);
              }
            }

            float leps  = std::min(std::max(ldiff[0][0], ldiff[0][1]), std::max(ldiff[1][2], ldiff[1][3]));
            float abeps = std::min(std::max(abdiff[0][0], abdiff[0][1]), std::max(abdiff[1][2], abdiff[1][3]));

            for (int d = 0; d < 2; d++)
            {
              homo[d][tr][tc] = 0;
              for (int i = 0; i < 4; i++)
              {
                homo[d][tr][tc] += (ldiff[d][i] <= leps) * (abdiff[d][i] <= abeps);
              }
            }
          }
        }

        //  Combine the most homogeneous pixels for the final result:
        for (int row = top + 3; row < top + TS - 3 && row < height - 5; row++)
        {
          int tr = row - top;

          for (int col = left + 3, tc = 3; col < std::min(left + TS - 3, width - 5); col++, tc++)
          {
            uint16_t hm0 = 0, hm1 = 0;
            for (int i = tr - 1; i <= tr + 1; i++)
              for (int j = tc - 1; j <= tc + 1; j++)
              {
                hm0 += homo[0][i][j];
                hm1 += homo[1][i][j];
              }

            if (hm0 != hm1)
            {
              int dir                  = hm1 > hm0;
              red[row * width + col]   = rgb[dir][tr][tc][0];
              green[row * width + col] = rgb[dir][tr][tc][1];
              blue[row * width + col]  = rgb[dir][tr][tc][2];
            }
            else
            {
              red[row * width + col]   = 0.5f * (rgb[0][tr][tc][0] + rgb[1][tr][tc][0]);
              green[row * width + col] = 0.5f * (rgb[0][tr][tc][1] + rgb[1][tr][tc][1]);
              blue[row * width + col]  = 0.5f * (rgb[0][tr][tc][2] + rgb[1][tr][tc][2]);
            }
          }
        }

        //             if(plistener) {
        //                 progresscounter++;

        //                 if(progresscounter % 32 == 0) {
        // #ifdef _OPENMP
        //                     #pragma omp critical (ahdprogress)
        // #endif
        //                     {
        //                         progress += 32.0 * SQR(TS - 6) / (height * width);
        //                         progress = std::min(progress, 1.0);
        //                         plistener->setProgress(progress);
        //                     }
        //                 }
        //             }
      }
    }
    delete[] buffer;
  }
  // if(plistener) {
  //     plistener->setProgress (1.0);
  // }
}


extern "C" void
ahd_demosaic(const float *rawData, float *debayered_image, size_t width, size_t height, unsigned int filters)
{
  float *debayered_r_temp = new float[width * height];
  float *debayered_g_temp = new float[width * height];
  float *debayered_b_temp = new float[width * height];

  ahd_demosaic_rgb(rawData, debayered_r_temp, debayered_g_temp, debayered_b_temp, width, height, filters);

  for (size_t row = 0; row < height; row++)
  {
    for (size_t col = 0; col < width; col++)
    {
      debayered_image[3 * (row * width + col) + 0] = debayered_r_temp[row * width + col];
      debayered_image[3 * (row * width + col) + 1] = debayered_g_temp[row * width + col];
      debayered_image[3 * (row * width + col) + 2] = debayered_b_temp[row * width + col];
    }
  }

  delete[] debayered_r_temp;
  delete[] debayered_g_temp;
  delete[] debayered_b_temp;
}



/**
 * RATIO CORRECTED DEMOSAICING
 * Luis Sanz Rodriguez (luis.sanz.rodriguez(at)gmail(dot)com)
 *
 * Release 2.3 @ 171125
 *
 * Original code from https://github.com/LuisSR/RCD-Demosaicing
 * Licensed under the GNU GPL version 3
 */
// Tiled version by Ingo Weyrich (heckflosse67@gmx.de)
extern "C" void
rcd_demosaic_rgb(const float *rawData, float *red, float *green, float *blue, size_t w, size_t h, unsigned int filters)
{
  const int width  = w;
  const int height = h;
  // Test for RGB cfa
  // for (int i = 0; i < 2; i++) {
  //     for (int j = 0; j < 2; j++) {
  //         if (FC(i, j) == 3) {
  //             // avoid crash
  //             std::cout << "rcd_demosaic supports only RGB Colour filter arrays. Falling back to igv_interpolate" << std::endl;
  //             igv_interpolate(W, H);
  //             return;
  //         }
  //     }
  // }

  // std::unique_ptr<StopWatch> stop;

  // if (measure) {
  //     std::cout << "Demosaicing " << W << "x" << H << " image using rcd with " << chunkSize << " tiles per thread" << std::endl;
  //     stop.reset(new StopWatch("rcd demosaic"));
  // }

  // double progress = 0.0;

  // if (plistener) {
  //     plistener->setProgressStr(Glib::ustring::compose(M("TP_RAW_DMETHOD_PROGRESSBAR"), M("TP_RAW_RCD")));
  //     plistener->setProgress(progress);
  // }

  const unsigned int cfarray[2][2] = {{FC(0, 0, filters), FC(0, 1, filters)}, {FC(1, 0, filters), FC(1, 1, filters)}};
  constexpr int      rcdBorder     = 9;
  constexpr int      tileSize      = 214;
  constexpr int      tileSizeN     = tileSize - 2 * rcdBorder;
  const int          numTh         = height / (tileSizeN) + ((height % (tileSizeN)) ? 1 : 0);
  const int          numTw         = width / (tileSizeN) + ((width % (tileSizeN)) ? 1 : 0);
  constexpr int      w1 = tileSize, w2 = 2 * tileSize, w3 = 3 * tileSize, w4 = 4 * tileSize;
  //Tolerance to avoid dividing by zero
  constexpr float eps   = 1e-5f;
  constexpr float epssq = 1e-10f;

#ifdef _OPENMP
#pragma omp parallel
#endif
  {
    int    progresscounter           = 0;
    float *cfa                       = (float *)calloc(tileSize * tileSize, sizeof *cfa);
    float(*rgb)[tileSize * tileSize] = (float(*)[tileSize * tileSize]) malloc(3 * sizeof *rgb);
    float *VH_Dir                    = (float *)calloc(tileSize * tileSize, sizeof *VH_Dir);
    float *PQ_Dir                    = (float *)calloc(tileSize * tileSize, sizeof *PQ_Dir);
    float *lpf                       = PQ_Dir;   // reuse buffer, they don't overlap in usage

#ifdef _OPENMP
    #pragma omp for schedule(dynamic, 32) collapse(2) nowait
#endif
    for (int tr = 0; tr < numTh; ++tr)
    {
      for (int tc = 0; tc < numTw; ++tc)
      {
        const int rowStart = tr * tileSizeN;
        const int rowEnd   = std::min(rowStart + tileSize, height);
        if (rowStart + rcdBorder == rowEnd - rcdBorder)
        {
          continue;
        }
        const int colStart = tc * tileSizeN;
        const int colEnd   = std::min(colStart + tileSize, width);
        if (colStart + rcdBorder == colEnd - rcdBorder)
        {
          continue;
        }

        const int tileRows = std::min(rowEnd - rowStart, tileSize);
        const int tilecols = std::min(colEnd - colStart, tileSize);

        for (int row = rowStart; row < rowEnd; row++)
        {
          int indx = (row - rowStart) * tileSize;
          int c0   = fc(cfarray, row, colStart);
          int c1   = fc(cfarray, row, colStart + 1);
          int col  = colStart;

          for (; col < colEnd - 1; col += 2, indx += 2)
          {
            cfa[indx] = rgb[c0][indx] = LIM(rawData[row * width + col] / 65535.f, 0.f, 1.f);
            cfa[indx + 1] = rgb[c1][indx + 1] = LIM(rawData[row * width + col + 1] / 65535.f, 0.f, 1.0f);
          }
          if (col < colEnd)
          {
            cfa[indx] = rgb[c0][indx] = LIM(rawData[row * width + col], 0.f, 1.f);
          }
        }

        /**
         * STEP 1: Find cardinal and diagonal interpolation directions
         */

        for (int row = 4; row < tileRows - 4; row++)
        {
          for (int col = 4, indx = row * tileSize + col; col < tilecols - 4; col++, indx++)
          {
            const float cfai = cfa[indx];
            //Calculate h/v local discrimination
            float V_Stat = std::max(
                epssq,
                -18.f * cfai
                        * (cfa[indx - w1] + cfa[indx + w1] + 2.f * (cfa[indx - w2] + cfa[indx + w2]) - cfa[indx - w3]
                           - cfa[indx + w3])
                    - 2.f * cfai * (cfa[indx - w4] + cfa[indx + w4] - 19.f * cfai)
                    - cfa[indx - w1]
                          * (70.f * cfa[indx + w1] + 12.f * cfa[indx - w2] - 24.f * cfa[indx + w2]
                             + 38.f * cfa[indx - w3] - 16.f * cfa[indx + w3] - 12.f * cfa[indx - w4]
                             + 6.f * cfa[indx + w4] - 46.f * cfa[indx - w1])
                    + cfa[indx + w1]
                          * (24.f * cfa[indx - w2] - 12.f * cfa[indx + w2] + 16.f * cfa[indx - w3]
                             - 38.f * cfa[indx + w3] - 6.f * cfa[indx - w4] + 12.f * cfa[indx + w4]
                             + 46.f * cfa[indx + w1])
                    + cfa[indx - w2]
                          * (14.f * cfa[indx + w2] - 12.f * cfa[indx + w3] - 2.f * cfa[indx - w4] + 2.f * cfa[indx + w4]
                             + 11.f * cfa[indx - w2])
                    + cfa[indx + w2]
                          * (-12.f * cfa[indx - w3] + 2.f * (cfa[indx - w4] - cfa[indx + w4]) + 11.f * cfa[indx + w2])
                    + cfa[indx - w3] * (2.f * cfa[indx + w3] - 6.f * cfa[indx - w4] + 10.f * cfa[indx - w3])
                    + cfa[indx + w3] * (-6.f * cfa[indx + w4] + 10.f * cfa[indx + w3]) + cfa[indx - w4] * cfa[indx - w4]
                    + cfa[indx + w4] * cfa[indx + w4]);
            float H_Stat = std::max(
                epssq,
                -18.f * cfai
                        * (cfa[indx - 1] + cfa[indx + 1] + 2.f * (cfa[indx - 2] + cfa[indx + 2]) - cfa[indx - 3]
                           - cfa[indx + 3])
                    - 2.f * cfai * (cfa[indx - 4] + cfa[indx + 4] - 19.f * cfai)
                    - cfa[indx - 1]
                          * (70.f * cfa[indx + 1] + 12.f * cfa[indx - 2] - 24.f * cfa[indx + 2] + 38.f * cfa[indx - 3]
                             - 16.f * cfa[indx + 3] - 12.f * cfa[indx - 4] + 6.f * cfa[indx + 4] - 46.f * cfa[indx - 1])
                    + cfa[indx + 1]
                          * (24.f * cfa[indx - 2] - 12.f * cfa[indx + 2] + 16.f * cfa[indx - 3] - 38.f * cfa[indx + 3]
                             - 6.f * cfa[indx - 4] + 12.f * cfa[indx + 4] + 46.f * cfa[indx + 1])
                    + cfa[indx - 2]
                          * (14.f * cfa[indx + 2] - 12.f * cfa[indx + 3] - 2.f * cfa[indx - 4] + 2.f * cfa[indx + 4]
                             + 11.f * cfa[indx - 2])
                    + cfa[indx + 2]
                          * (-12.f * cfa[indx - 3] + 2.f * (cfa[indx - 4] - cfa[indx + 4]) + 11.f * cfa[indx + 2])
                    + cfa[indx - 3] * (2.f * cfa[indx + 3] - 6.f * cfa[indx - 4] + 10.f * cfa[indx - 3])
                    + cfa[indx + 3] * (-6.f * cfa[indx + 4] + 10.f * cfa[indx + 3]) + cfa[indx - 4] * cfa[indx - 4]
                    + cfa[indx + 4] * cfa[indx + 4]);

            VH_Dir[indx] = V_Stat / (V_Stat + H_Stat);
          }
        }

        /**
         * STEP 2: Calculate the low pass filter
         */
        // Step 2.1: Low pass filter incorporating green, red and blue local samples from the raw data

        for (int row = 2; row < tileRows - 2; row++)
        {
          for (int col = 2 + (fc(cfarray, row, 0) & 1), indx = row * tileSize + col; col < tilecols - 2;
               col += 2, indx += 2)
          {
            lpf[indx >> 1]
                = 0.25f * cfa[indx] + 0.125f * (cfa[indx - w1] + cfa[indx + w1] + cfa[indx - 1] + cfa[indx + 1])
                  + 0.0625f * (cfa[indx - w1 - 1] + cfa[indx - w1 + 1] + cfa[indx + w1 - 1] + cfa[indx + w1 + 1]);
          }
        }

        /**
         * STEP 3: Populate the green channel
         */
        // Step 3.1: Populate the green channel at blue and red CFA positions
        for (int row = 4; row < tileRows - 4; row++)
        {
          int col  = 4 + (fc(cfarray, row, 0) & 1);
          int indx = row * tileSize + col;
#ifdef __SSE2__
          const vfloat zd5v  = F2V(0.5f);
          const vfloat zd25v = F2V(0.25f);
          const vfloat epsv  = F2V(eps);
          for (; col < tilecols - 7; col += 8, indx += 8)
          {
            // Cardinal gradients
            const vfloat cfai = LC2VFU(cfa[indx]);
            const vfloat N_Grad
                = epsv + (vabsf(LC2VFU(cfa[indx - w1]) - LC2VFU(cfa[indx + w1])) + vabsf(cfai - LC2VFU(cfa[indx - w2])))
                  + (vabsf(LC2VFU(cfa[indx - w1]) - LC2VFU(cfa[indx - w3]))
                     + vabsf(LC2VFU(cfa[indx - w2]) - LC2VFU(cfa[indx - w4])));
            const vfloat S_Grad
                = epsv + (vabsf(LC2VFU(cfa[indx - w1]) - LC2VFU(cfa[indx + w1])) + vabsf(cfai - LC2VFU(cfa[indx + w2])))
                  + (vabsf(LC2VFU(cfa[indx + w1]) - LC2VFU(cfa[indx + w3]))
                     + vabsf(LC2VFU(cfa[indx + w2]) - LC2VFU(cfa[indx + w4])));
            const vfloat W_Grad
                = epsv + (vabsf(LC2VFU(cfa[indx - 1]) - LC2VFU(cfa[indx + 1])) + vabsf(cfai - LC2VFU(cfa[indx - 2])))
                  + (vabsf(LC2VFU(cfa[indx - 1]) - LC2VFU(cfa[indx - 3]))
                     + vabsf(LC2VFU(cfa[indx - 2]) - LC2VFU(cfa[indx - 4])));
            const vfloat E_Grad
                = epsv + (vabsf(LC2VFU(cfa[indx - 1]) - LC2VFU(cfa[indx + 1])) + vabsf(cfai - LC2VFU(cfa[indx + 2])))
                  + (vabsf(LC2VFU(cfa[indx + 1]) - LC2VFU(cfa[indx + 3]))
                     + vabsf(LC2VFU(cfa[indx + 2]) - LC2VFU(cfa[indx + 4])));

            // Cardinal pixel estimations
            const vfloat lpfi  = LVFU(lpf[indx >> 1]);
            const vfloat N_Est = LC2VFU(cfa[indx - w1])
                                 + (LC2VFU(cfa[indx - w1]) * (lpfi - LVFU(lpf[(indx - w2) >> 1]))
                                    / (epsv + lpfi + LVFU(lpf[(indx - w2) >> 1])));
            const vfloat S_Est = LC2VFU(cfa[indx + w1])
                                 + (LC2VFU(cfa[indx + w1]) * (lpfi - LVFU(lpf[(indx + w2) >> 1]))
                                    / (epsv + lpfi + LVFU(lpf[(indx + w2) >> 1])));
            const vfloat W_Est = LC2VFU(cfa[indx - 1])
                                 + (LC2VFU(cfa[indx - 1]) * (lpfi - LVFU(lpf[(indx - 2) >> 1]))
                                    / (epsv + lpfi + LVFU(lpf[(indx - 2) >> 1])));
            const vfloat E_Est = LC2VFU(cfa[indx + 1])
                                 + (LC2VFU(cfa[indx + 1]) * (lpfi - LVFU(lpf[(indx + 2) >> 1]))
                                    / (epsv + lpfi + LVFU(lpf[(indx + 2) >> 1])));

            // Vertical and horizontal estimations
            const vfloat V_Est = (S_Grad * N_Est + N_Grad * S_Est) / (N_Grad + S_Grad);
            const vfloat H_Est = (W_Grad * E_Est + E_Grad * W_Est) / (E_Grad + W_Grad);

            // G@B and G@R interpolation
            // Refined vertical and horizontal local discrimination
            const vfloat VH_Central_Value       = LC2VFU(VH_Dir[indx]);
            const vfloat VH_Neighbourhood_Value = zd25v
                                                  * ((LC2VFU(VH_Dir[indx - w1 - 1]) + LC2VFU(VH_Dir[indx - w1 + 1]))
                                                     + (LC2VFU(VH_Dir[indx + w1 - 1]) + LC2VFU(VH_Dir[indx + w1 + 1])));

#  if defined(__clang__)
            const vfloat VH_Disc = vself(
                vmaskf_lt(vabsf(zd5v - VH_Central_Value), vabsf(zd5v - VH_Neighbourhood_Value)),
                VH_Neighbourhood_Value,
                VH_Central_Value);
#  else
            const vfloat VH_Disc = vabsf(zd5v - VH_Central_Value) < vabsf(zd5v - VH_Neighbourhood_Value)
                                       ? VH_Neighbourhood_Value
                                       : VH_Central_Value;
#  endif
            STC2VFU(rgb[1][indx], vintpf(VH_Disc, H_Est, V_Est));
          }
#endif
          for (; col < tilecols - 4; col += 2, indx += 2)
          {
            // Cardinal gradients
            const float cfai = cfa[indx];
            const float N_Grad
                = eps + (std::fabs(cfa[indx - w1] - cfa[indx + w1]) + std::fabs(cfai - cfa[indx - w2]))
                  + (std::fabs(cfa[indx - w1] - cfa[indx - w3]) + std::fabs(cfa[indx - w2] - cfa[indx - w4]));
            const float S_Grad
                = eps + (std::fabs(cfa[indx - w1] - cfa[indx + w1]) + std::fabs(cfai - cfa[indx + w2]))
                  + (std::fabs(cfa[indx + w1] - cfa[indx + w3]) + std::fabs(cfa[indx + w2] - cfa[indx + w4]));
            const float W_Grad
                = eps + (std::fabs(cfa[indx - 1] - cfa[indx + 1]) + std::fabs(cfai - cfa[indx - 2]))
                  + (std::fabs(cfa[indx - 1] - cfa[indx - 3]) + std::fabs(cfa[indx - 2] - cfa[indx - 4]));
            const float E_Grad
                = eps + (std::fabs(cfa[indx - 1] - cfa[indx + 1]) + std::fabs(cfai - cfa[indx + 2]))
                  + (std::fabs(cfa[indx + 1] - cfa[indx + 3]) + std::fabs(cfa[indx + 2] - cfa[indx + 4]));

            // Cardinal pixel estimations
            const float lpfi = lpf[indx >> 1];
            const float N_Est
                = cfa[indx - w1] * (1.f + (lpfi - lpf[(indx - w2) >> 1]) / (eps + lpfi + lpf[(indx - w2) >> 1]));
            const float S_Est
                = cfa[indx + w1] * (1.f + (lpfi - lpf[(indx + w2) >> 1]) / (eps + lpfi + lpf[(indx + w2) >> 1]));
            const float W_Est
                = cfa[indx - 1] * (1.f + (lpfi - lpf[(indx - 2) >> 1]) / (eps + lpfi + lpf[(indx - 2) >> 1]));
            const float E_Est
                = cfa[indx + 1] * (1.f + (lpfi - lpf[(indx + 2) >> 1]) / (eps + lpfi + lpf[(indx + 2) >> 1]));

            // Vertical and horizontal estimations
            const float V_Est = (S_Grad * N_Est + N_Grad * S_Est) / (N_Grad + S_Grad);
            const float H_Est = (W_Grad * E_Est + E_Grad * W_Est) / (E_Grad + W_Grad);

            // G@B and G@R interpolation
            // Refined vertical and horizontal local discrimination
            const float VH_Central_Value = VH_Dir[indx];
            const float VH_Neighbourhood_Value
                = 0.25f
                  * ((VH_Dir[indx - w1 - 1] + VH_Dir[indx - w1 + 1]) + (VH_Dir[indx + w1 - 1] + VH_Dir[indx + w1 + 1]));

            const float VH_Disc = std::fabs(0.5f - VH_Central_Value) < std::fabs(0.5f - VH_Neighbourhood_Value)
                                      ? VH_Neighbourhood_Value
                                      : VH_Central_Value;
            rgb[1][indx] = VH_Disc * H_Est + (1.f - VH_Disc) * V_Est;
          }
        }

        /**
         * STEP 4: Populate the red and blue channels
         */

        // Step 4.1: Calculate P/Q diagonal local discrimination
        for (int row = rcdBorder - 4; row < tileRows - rcdBorder + 4; row++)
        {
          for (int col = rcdBorder - 4 + (fc(cfarray, row, rcdBorder) & 1), indx = row * tileSize + col;
               col < tilecols - rcdBorder + 4;
               col += 2, indx += 2)
          {
            const float cfai = cfa[indx];

            float P_Stat = std::max(
                epssq,
                -18.f * cfai
                        * (cfa[indx - w1 - 1] + cfa[indx + w1 + 1] + 2.f * (cfa[indx - w2 - 2] + cfa[indx + w2 + 2])
                           - cfa[indx - w3 - 3] - cfa[indx + w3 + 3])
                    - 2.f * cfai * (cfa[indx - w4 - 4] + cfa[indx + w4 + 4] - 19.f * cfai)
                    - cfa[indx - w1 - 1]
                          * (70.f * cfa[indx + w1 + 1] - 12.f * cfa[indx - w2 - 2] + 24.f * cfa[indx + w2 + 2]
                             - 38.f * cfa[indx - w3 - 3] + 16.f * cfa[indx + w3 + 3] + 12.f * cfa[indx - w4 - 4]
                             - 6.f * cfa[indx + w4 + 4] + 46.f * cfa[indx - w1 - 1])
                    + cfa[indx + w1 + 1]
                          * (24.f * cfa[indx - w2 - 2] - 12.f * cfa[indx + w2 + 2] + 16.f * cfa[indx - w3 - 3]
                             - 38.f * cfa[indx + w3 + 3] - 6.f * cfa[indx - w4 - 4] + 12.f * cfa[indx + w4 + 4]
                             + 46.f * cfa[indx + w1 + 1])
                    + cfa[indx - w2 - 2]
                          * (14.f * cfa[indx + w2 + 2] - 12.f * cfa[indx + w3 + 3]
                             - 2.f * (cfa[indx - w4 - 4] - cfa[indx + w4 + 4]) + 11.f * cfa[indx - w2 - 2])
                    - cfa[indx + w2 + 2]
                          * (12.f * cfa[indx - w3 - 3] + 2.f * (cfa[indx - w4 - 4] - cfa[indx + w4 + 4])
                             + 11.f * cfa[indx + w2 + 2])
                    + cfa[indx - w3 - 3]
                          * (2.f * cfa[indx + w3 + 3] - 6.f * cfa[indx - w4 - 4] + 10.f * cfa[indx - w3 - 3])
                    - cfa[indx + w3 + 3] * (6.f * cfa[indx + w4 + 4] + 10.f * cfa[indx + w3 + 3])
                    + cfa[indx - w4 - 4] * cfa[indx - w4 - 4] + cfa[indx + w4 + 4] * cfa[indx + w4 + 4]);
            float Q_Stat = std::max(
                epssq,
                -18.f * cfai
                        * (cfa[indx + w1 - 1] + cfa[indx - w1 + 1] + 2.f * (cfa[indx + w2 - 2] + cfa[indx - w2 + 2])
                           - cfa[indx + w3 - 3] - cfa[indx - w3 + 3])
                    - 2.f * cfai * (cfa[indx + w4 - 4] + cfa[indx - w4 + 4] - 19.f * cfai)
                    - cfa[indx + w1 - 1]
                          * (70.f * cfa[indx - w1 + 1] - 12.f * cfa[indx + w2 - 2] + 24.f * cfa[indx - w2 + 2]
                             - 38.f * cfa[indx + w3 - 3] + 16.f * cfa[indx - w3 + 3] + 12.f * cfa[indx + w4 - 4]
                             - 6.f * cfa[indx - w4 + 4] + 46.f * cfa[indx + w1 - 1])
                    + cfa[indx - w1 + 1]
                          * (24.f * cfa[indx + w2 - 2] - 12.f * cfa[indx - w2 + 2] + 16.f * cfa[indx + w3 - 3]
                             - 38.f * cfa[indx - w3 + 3] - 6.f * cfa[indx + w4 - 4] + 12.f * cfa[indx - w4 + 4]
                             + 46.f * cfa[indx - w1 + 1])
                    + cfa[indx + w2 - 2]
                          * (14.f * cfa[indx - w2 + 2] - 12.f * cfa[indx - w3 + 3]
                             - 2.f * (cfa[indx + w4 - 4] - cfa[indx - w4 + 4]) + 11.f * cfa[indx + w2 - 2])
                    - cfa[indx - w2 + 2]
                          * (12.f * cfa[indx + w3 - 3] + 2.f * (cfa[indx + w4 - 4] - cfa[indx - w4 + 4])
                             + 11.f * cfa[indx - w2 + 2])
                    + cfa[indx + w3 - 3]
                          * (2.f * cfa[indx - w3 + 3] - 6.f * cfa[indx + w4 - 4] + 10.f * cfa[indx + w3 - 3])
                    - cfa[indx - w3 + 3] * (6.f * cfa[indx - w4 + 4] + 10.f * cfa[indx - w3 + 3])
                    + cfa[indx + w4 - 4] * cfa[indx + w4 - 4] + cfa[indx - w4 + 4] * cfa[indx - w4 + 4]);

            PQ_Dir[indx] = P_Stat / (P_Stat + Q_Stat);
          }
        }

        // Step 4.2: Populate the red and blue channels at blue and red CFA positions
        for (int row = rcdBorder - 3; row < tileRows - rcdBorder + 3; row++)
        {
          for (int col  = rcdBorder - 3 + (fc(cfarray, row, rcdBorder - 1) & 1),
                   indx = row * tileSize + col,
                   c    = 2 - fc(cfarray, row, col);
               col < tilecols - rcdBorder + 3;
               col += 2, indx += 2)
          {
            // Refined P/Q diagonal local discrimination
            float PQ_Central_Value = PQ_Dir[indx];
            float PQ_Neighbourhood_Value
                = 0.25f
                  * (PQ_Dir[indx - w1 - 1] + PQ_Dir[indx - w1 + 1] + PQ_Dir[indx + w1 - 1] + PQ_Dir[indx + w1 + 1]);

            float PQ_Disc = (std::fabs(0.5f - PQ_Central_Value) < std::fabs(0.5f - PQ_Neighbourhood_Value))
                                ? PQ_Neighbourhood_Value
                                : PQ_Central_Value;

            // Diagonal gradients
            float NW_Grad = eps + std::fabs(rgb[c][indx - w1 - 1] - rgb[c][indx + w1 + 1])
                            + std::fabs(rgb[c][indx - w1 - 1] - rgb[c][indx - w3 - 3])
                            + std::fabs(rgb[1][indx] - rgb[1][indx - w2 - 2]);
            float NE_Grad = eps + std::fabs(rgb[c][indx - w1 + 1] - rgb[c][indx + w1 - 1])
                            + std::fabs(rgb[c][indx - w1 + 1] - rgb[c][indx - w3 + 3])
                            + std::fabs(rgb[1][indx] - rgb[1][indx - w2 + 2]);
            float SW_Grad = eps + std::fabs(rgb[c][indx - w1 + 1] - rgb[c][indx + w1 - 1])
                            + std::fabs(rgb[c][indx + w1 - 1] - rgb[c][indx + w3 - 3])
                            + std::fabs(rgb[1][indx] - rgb[1][indx + w2 - 2]);
            float SE_Grad = eps + std::fabs(rgb[c][indx - w1 - 1] - rgb[c][indx + w1 + 1])
                            + std::fabs(rgb[c][indx + w1 + 1] - rgb[c][indx + w3 + 3])
                            + std::fabs(rgb[1][indx] - rgb[1][indx + w2 + 2]);

            // Diagonal colour differences
            float NW_Est = rgb[c][indx - w1 - 1] - rgb[1][indx - w1 - 1];
            float NE_Est = rgb[c][indx - w1 + 1] - rgb[1][indx - w1 + 1];
            float SW_Est = rgb[c][indx + w1 - 1] - rgb[1][indx + w1 - 1];
            float SE_Est = rgb[c][indx + w1 + 1] - rgb[1][indx + w1 + 1];

            // P/Q estimations
            float P_Est = (NW_Grad * SE_Est + SE_Grad * NW_Est) / (NW_Grad + SE_Grad);
            float Q_Est = (NE_Grad * SW_Est + SW_Grad * NE_Est) / (NE_Grad + SW_Grad);

            // R@B and B@R interpolation
            rgb[c][indx] = rgb[1][indx] + (1.f - PQ_Disc) * P_Est + PQ_Disc * Q_Est;
          }
        }

        // Step 4.3: Populate the red and blue channels at green CFA positions
        for (int row = rcdBorder; row < tileRows - rcdBorder; row++)
        {
          for (int col = rcdBorder + (fc(cfarray, row, rcdBorder - 1) & 1), indx = row * tileSize + col;
               col < tilecols - rcdBorder;
               col += 2, indx += 2)
          {
            // Refined vertical and horizontal local discrimination
            float VH_Central_Value = VH_Dir[indx];
            float VH_Neighbourhood_Value
                = 0.25f
                  * ((VH_Dir[indx - w1 - 1] + VH_Dir[indx - w1 + 1]) + (VH_Dir[indx + w1 - 1] + VH_Dir[indx + w1 + 1]));

            float VH_Disc = (std::fabs(0.5f - VH_Central_Value) < std::fabs(0.5f - VH_Neighbourhood_Value))
                                ? VH_Neighbourhood_Value
                                : VH_Central_Value;
            float rgb1 = rgb[1][indx];
            float N1   = eps + std::fabs(rgb1 - rgb[1][indx - w2]);
            float S1   = eps + std::fabs(rgb1 - rgb[1][indx + w2]);
            float W1   = eps + std::fabs(rgb1 - rgb[1][indx - 2]);
            float E1   = eps + std::fabs(rgb1 - rgb[1][indx + 2]);

            float rgb1mw1 = rgb[1][indx - w1];
            float rgb1pw1 = rgb[1][indx + w1];
            float rgb1m1  = rgb[1][indx - 1];
            float rgb1p1  = rgb[1][indx + 1];
            for (int c = 0; c <= 2; c += 2)
            {
              // Cardinal gradients
              float SNabs  = std::fabs(rgb[c][indx - w1] - rgb[c][indx + w1]);
              float EWabs  = std::fabs(rgb[c][indx - 1] - rgb[c][indx + 1]);
              float N_Grad = N1 + SNabs + std::fabs(rgb[c][indx - w1] - rgb[c][indx - w3]);
              float S_Grad = S1 + SNabs + std::fabs(rgb[c][indx + w1] - rgb[c][indx + w3]);
              float W_Grad = W1 + EWabs + std::fabs(rgb[c][indx - 1] - rgb[c][indx - 3]);
              float E_Grad = E1 + EWabs + std::fabs(rgb[c][indx + 1] - rgb[c][indx + 3]);

              // Cardinal colour differences
              float N_Est = rgb[c][indx - w1] - rgb1mw1;
              float S_Est = rgb[c][indx + w1] - rgb1pw1;
              float W_Est = rgb[c][indx - 1] - rgb1m1;
              float E_Est = rgb[c][indx + 1] - rgb1p1;

              // Vertical and horizontal estimations
              float V_Est = (N_Grad * S_Est + S_Grad * N_Est) / (N_Grad + S_Grad);
              float H_Est = (E_Grad * W_Est + W_Grad * E_Est) / (E_Grad + W_Grad);

              // R@G and B@G interpolation
              rgb[c][indx] = rgb1 + (1.f - VH_Disc) * V_Est + VH_Disc * H_Est;
            }
          }
        }

        for (int row = rowStart + rcdBorder; row < rowEnd - rcdBorder; ++row)
        {
          for (int col = colStart + rcdBorder; col < colEnd - rcdBorder; ++col)
          {
            int idx                  = (row - rowStart) * tileSize + col - colStart;
            red[row * width + col]   = std::max(0.f, rgb[0][idx] * 65535.f);
            green[row * width + col] = std::max(0.f, rgb[1][idx] * 65535.f);
            blue[row * width + col]  = std::max(0.f, rgb[2][idx] * 65535.f);
          }
        }

        //             if (plistener) {
        //                 progresscounter++;
        //                 if (progresscounter % 32 == 0) {
        // #ifdef _OPENMP
        //                     #pragma omp critical (rcdprogress)
        // #endif
        //                     {
        //                         progress += (double)32 * ((tileSizeN) * (tileSizeN)) / (H * W);
        //                         progress = progress > 1.0 ? 1.0 : progress;
        //                         plistener->setProgress(progress);
        //                     }
        //                 }
        //             }
      }
    }

    free(cfa);
    free(rgb);
    free(VH_Dir);
    free(PQ_Dir);
  }

  border_interpolate(width, height, rcdBorder, rawData, red, green, blue, filters);

  // if (plistener) {
  //     plistener->setProgress(1);
  // }
}


extern "C" void
rcd_demosaic(const float *rawData, float *debayered_image, size_t width, size_t height, unsigned int filters)
{
  float *debayered_r_temp = new float[width * height];
  float *debayered_g_temp = new float[width * height];
  float *debayered_b_temp = new float[width * height];

  rcd_demosaic_rgb(rawData, debayered_r_temp, debayered_g_temp, debayered_b_temp, width, height, filters);

  for (size_t row = 0; row < height; row++)
  {
    for (size_t col = 0; col < width; col++)
    {
      debayered_image[3 * (row * width + col) + 0] = debayered_r_temp[row * width + col];
      debayered_image[3 * (row * width + col) + 1] = debayered_g_temp[row * width + col];
      debayered_image[3 * (row * width + col) + 2] = debayered_b_temp[row * width + col];
    }
  }

  delete[] debayered_r_temp;
  delete[] debayered_g_temp;
  delete[] debayered_b_temp;
}
