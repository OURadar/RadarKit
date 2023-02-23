//
//  RKSIMD.h
//  RadarKit
//
//  Created by Boonleng Cheong on 10/18/16.
//
//

#ifndef __RadarKit_SIMD__
#define __RadarKit_SIMD__

#include <RadarKit/RKFoundation.h>

#if defined(_MSC_VER)
// Microsoft C/C++-compatible compiler
#include <intrin.h>
#elif defined(_EXPLICIT_INTRINSIC)
// Old OSs that do not have x86intrin.h
#include <pmmintrin.h>
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
// GCC-compatible compiler, targeting x86/x86-64
#include <x86intrin.h>
#elif defined(__GNUC__) && defined(__ARM_NEON__)
// GCC-compatible compiler, targeting ARM with NEON
#include <arm_neon.h>
#elif defined(__GNUC__) && defined(__IWMMXT__)
// GCC-compatible compiler, targeting ARM with WMMX
#include <mmintrin.h>
#elif (defined(__GNUC__) || defined(__xlC__)) && (defined(__VEC__) || defined(__ALTIVEC__))
// XLC or GCC-compatible compiler, targeting PowerPC with VMX/VSX
#include <altivec.h>
#elif defined(__GNUC__) && defined(__SPE__)
// GCC-compatible compiler, targeting PowerPC with SPE
#include <spe.h>
#endif

#if defined(__AVX512F__)

// NOTE: Still need to define a lot of _mmXXX_XXX_pd equivalence for double precision calculations

typedef __m512 RKVec;
typedef __m256i RKVecCvt;
#define _rk_mm_add_pf(a, b)          _mm512_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm512_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm512_mul_ps(a, b)
#define _rk_mm_div_pf(a, b)          _mm512_div_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm512_set1_ps(a)
//#define _rk_mm_set_pf(a, b, c, d)    _mm512_set_ps(a, b, c, d, a, b, c, d, a, b, c, d, a, b, c, d)
#define _rk_mm_set_pf(v)             _mm512_set_ps(v[0], v[1], v[2], v[3], v[0], v[1], v[2], v[3], v[0], v[1], v[2], v[3], v[0], v[1], v[2], v[3])
#define _rk_mm_movehdup_pf(a)        _mm512_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm512_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm512_shuffle_ps(a, b, m)
//#define _rk_mm_fmaddsub_pf(a, b, c)  _mm512_fmaddsub_ps(a, b, c)
#define _rk_mm_cvtepi16_epi32(a)     _mm512_cvtepi16_epi32(a)                // AVX512
#define _rk_mm_cvtepi32_pf(a)        _mm512_cvtepi32_ps(a)                   // AVX512
#define _rk_mm_sqrt_pf(a)            _mm512_sqrt_ps(a)
#define _rk_mm_rcp_pf(a)             _mm512_rcp_ps(a)
#define _rk_mm_max_pf(a, b)          _mm512_max_ps(a, b)
#define _rk_mm_min_pf(a, b)          _mm512_min_ps(a, b)
//#if defined(_mm512_mul_ps)
//#define _rk_mm_log10_pf(a)           _mm512_log10_ps(a)
//#endif

#elif defined(__AVX__)

typedef __m256 RKVec;
typedef __m128i RKVecCvt;
#define _rk_mm_add_pf(a, b)          _mm256_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm256_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm256_mul_ps(a, b)
#define _rk_mm_div_pf(a, b)          _mm256_div_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm256_set1_ps(a)
//#define _rk_mm_set_pf(a, b, c, d)    _mm256_set_ps(a, b, c, d, a, b, c, d)
#define _rk_mm_set_pf(v)    _mm256_set_ps(v[0], v[1], v[2], v[3], v[0], v[1], v[2], v[3])
#define _rk_mm_movehdup_pf(a)        _mm256_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm256_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm256_shuffle_ps(a, b, m)
//#define _rk_mm_fmaddsub_pf(a, b, c)  _mm256_fmaddsub_ps(a, b, c)
//#define _rk_mm_fmsubadd_pf(a, b, c)  _mm256_fmsubadd_ps(a, b, c)
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm256_addsub_ps(_mm256_mul_ps(a, b), c)
#    if defined(__AVX2__)
#        define _rk_mm_cvtepi16_epi32(a)     _mm256_cvtepi16_epi32(a)                 // AVX2
#        define _rk_mm_cvtepi32_pf(a)        _mm256_cvtepi32_ps(a)                    // AVX
#    endif
#define _rk_mm_sqrt_pf(a)            _mm256_sqrt_ps(a)
#define _rk_mm_rcp_pf(a)             _mm256_rcp_ps(a)
#define _rk_mm_max_pf(a, b)          _mm256_max_ps(a, b)
#define _rk_mm_min_pf(a, b)          _mm256_min_ps(a, b)
//#if defined(_mm256_mul_ps)
//#define _rk_mm_log10_pf(a)           _mm256_log10_ps(a)
//#endif

#elif defined(_EXPLICIT_INTRINSIC)

typedef __m128 RKVec;
#define _rk_mm_add_pf(a, b)          _mm_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm_mul_ps(a, b)
#define _rk_mm_div_pf(a, b)          _mm_div_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm_set1_ps(a)
//#define _rk_mm_set_pf(a, b, c, d)    _mm_set_ps(a, b, c, d)
#define _rk_mm_set_pf(v)             _mm_set_ps(v[0], v[1], v[2], v[3])
#define _rk_mm_movehdup_pf(a)        _mm_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm_shuffle_ps(a, b, m)
//#define _rk_mm_fmadd_pf(a, b, c)     _mm_add_ps(_mm_mul_ps(a, b), c)         // FMA
//#define _rk_mm_fmadd_pf(a, b, c)     _mm_fmadd_ps(a, b, c)                   // FMA
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm_addsub_ps(_mm_mul_ps(a, b), c)      // SSE3
//#define _rk_mm_fmaddsub_pf(a, b, c)  _mm_fmaddsub_ps(a, b, c)                // FMA
//#define _rk_mm_fmsubadd_pf(a, b, c)  _mm_fmsubadd_ps(a, b, c)                // FMA
#define _rk_mm_unpacklo_epi16(a, b)  _mm_unpacklo_epi16(a, b)                // SSE2
#define _rk_mm_unpackhi_epi16(a, b)  _mm_unpackhi_epi16(a, b)                // SSE2
#define _rk_mm_cvtepi16_epi32(a)     _mm_cvtepi16_epi32(a)                   // SSE4.1
#define _rk_mm_cvtepi32_pf(a)        _mm_cvtepi32_ps(a)                      // SSE2
#define _rk_mm_sqrt_pf(a)            _mm_sqrt_ps(a)
#define _rk_mm_rcp_pf(a)             _mm_rcp_ps(a)
#define _rk_mm_max_pf(a, b)          _mm_max_ps(a, b)
#define _rk_mm_min_pf(a, b)          _mm_min_ps(a, b)
//#if defined(_mm_mul_ps)
//#define _rk_mm_log10_pf(a)           _mm_log10_ps(a)
//#endif

#elif defined(__ARM_NEON__)

typedef float32x4_t RKVec;
#define _rk_mm_add_pf(a, b)          vaddq_f32(a, b)
#define _rk_mm_sub_pf(a, b)          vsubq_f32(a, b)
#define _rk_mm_mul_pf(a, b)          vmulq_f32(a, b)
#define _rk_mm_div_pf(a, b)          vmulq_f32(a, vrecpeq_f32(b))
#define _rk_mm_set1_pf(a)            vld1q_dup_f32((float *)&a)
#define _rk_mm_set_pf(v)             vld1q_f32(v)
#define _rk_mm_movehdup_pf(a)        vaddq_f32(a, a)
#define _rk_mm_moveldup_pf(a)        vaddq_f32(a, a)
#define _rk_mm_shuffle_pf(a, b, m)   vaddq_f32(a, b)
#define _rk_mm_fmaddsub_pf(a, b, c)  vaddq_f32(a, b)
#define _rk_mm_unpacklo_epi16(a, b)  vaddq_f32(a, b)
#define _rk_mm_unpackhi_epi16(a, b)  vaddq_f32(a, b)
#define _rk_mm_cvtepi16_epi32(a)     vld1q_dup_f32(a)
#define _rk_mm_cvtepi32_pf(a)        vcvtq(a)
#define _rk_mm_sqrt_pf(a)            vsqrtq_f32(a)
#define _rk_mm_rcp_pf(a)             vsqrtq_f32(a)
#define _rk_mm_max_pf(a, b)          vaddq_f32(a, b)
#define _rk_mm_min_pf(a, b)          vaddq_f32(a, b)

#endif

void RKSIMD_show_info(void);
void RKSIMD_show_count(const int n);

void RKSIMD_mul(RKFloat *s1, RKFloat *s2, RKFloat *dst, const int n);

void RKSIMD_zcpy (RKIQZ *src, RKIQZ *dst, const int n);
void RKSIMD_zadd (RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n);
void RKSIMD_zsub (RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n);
void RKSIMD_zmul (RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n, const bool c);
void RKSIMD_zsmul(RKIQZ *src, RKIQZ *dst, const int n, const bool c);
void RKSIMD_izadd(RKIQZ *src, RKIQZ *dst, const int n);
void RKSIMD_izsub(RKIQZ *src, RKIQZ *dst, const int n);
void RKSIMD_izmul(RKIQZ *src, RKIQZ *dst, const int n, const bool c);
void RKSIMD_zcma (RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n, const bool c);
void RKSIMD_szcma(RKFloat *s1, RKIQZ *s2, RKIQZ *dst, const int n);
void RKSIMD_csz(RKFloat s, RKIQZ *src, RKIQZ *dst, const int n);
void RKSIMD_zscl (RKIQZ *src, const float f, RKIQZ *dst, const int n);
void RKSIMD_izscl(RKIQZ *srcdst, const float f, const int n);
void RKSIMD_zabs(RKIQZ *src, float *dst, const int n);
void RKSIMD_iymul(RKComplex *src, RKComplex *dst, const int n);
void RKSIMD_iymulc(RKComplex *src, RKComplex *dst, const int n);
void RKSIMD_iymul2(RKComplex *src, RKComplex *dst, const int n, const bool c);
void RKSIMD_iymul_reg(RKComplex *src, RKComplex *dst, const int n);
void RKSIMD_yconj(RKComplex *src, const int n);
void RKSIMD_ssadd(float *src, const float f, float *dst, const int n);
void RKSIMD_iyscl(RKComplex *src, const float s, const int n);

void RKSIMD_IQZ2Complex(RKIQZ *src, RKComplex *dst, const int n);
void RKSIMD_Complex2IQZ(RKComplex *src, RKIQZ *dst, const int n);
void RKSIMD_Int2Complex(RKInt16C *src, RKComplex *dst, const int n);
void RKSIMD_Int2Complex_reg(RKInt16C *src, RKComplex *dst, const int n);

void RKSIMD_subc(RKFloat *src, const RKFloat f, RKFloat *dst, const int n);
void RKSIMD_clamp(RKFloat *src, const RKFloat min, const RKFloat max, const int n);

void RKSIMD_izrmrm(RKIQZ *src, RKFloat *dst, RKFloat *x, RKFloat *y, RKFloat u, const int n);

#endif
