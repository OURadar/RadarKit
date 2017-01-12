//
//  RKSIMD.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/18/16.
//
//

#ifndef __RadarKit_RKSIMD__
#define __RadarKit_RKSIMD__

#include <RadarKit/RKFoundation.h>

#if defined(_MSC_VER)
// Microsoft C/C++-compatible compiler
#include <intrin.h>
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
#define _rk_mm_movehdup_pf(a)        _mm512_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm512_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm512_shuffle_ps(a, b, m)
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm512_fmaddsub_ps(a, b, c)
#define _rk_mm_setzero_si()          _mm512_setzero_si512()
#define _rk_mm_cvtepi16_epi32(a)     _mm512_cvtepi16_epi32(a)                // AVX512
#define _rk_mm_cvtepi32_pf(a)        _mm512_cvtepi32_ps(a)                   // AVX512
#define _rk_mm_sqrt_pf(a)            _mm512_sqrt_ps(a)
#if defined(_mm512_mul_ps)
#define _rk_mm_log10_pf(a)           _mm512_log10_ps(a)
#endif

#elif defined(__AVX__)

typedef __m256 RKVec;
typedef __m128i RKVecCvt;
#define _rk_mm_add_pf(a, b)          _mm256_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm256_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm256_mul_ps(a, b)
#define _rk_mm_div_pf(a, b)          _mm256_div_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm256_set1_ps(a)
#define _rk_mm_movehdup_pf(a)        _mm256_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm256_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm256_shuffle_ps(a, b, m)
//#define _rk_mm_fmaddsub_pf(a, b, c)  _mm256_fmaddsub_ps(a, b, c)
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm256_addsub_ps(_mm256_mul_ps(a, b), c)
#define _rk_mm_setzero_si()          _mm256_setzero_si256()
#    if defined(__AVX2__)
#        define _rk_mm_cvtepi16_epi32(a)     _mm256_cvtepi16_epi32(a)                 // AVX2
#        define _rk_mm_cvtepi32_pf(a)        _mm256_cvtepi32_ps(a)                    // AVX
#    endif
#define _rk_mm_sqrt_pf(a)            _mm256_sqrt_ps(a)
#if defined(_mm256_mul_ps)
#define _rk_mm_log10_pf(a)           _mm256_log10_ps(a)
#endif

#else

typedef __m128 RKVec;
#define _rk_mm_add_pf(a, b)          _mm_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm_mul_ps(a, b)
#define _rk_mm_div_pf(a, b)          _mm_div_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm_set1_ps(a)
#define _rk_mm_movehdup_pf(a)        _mm_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm_shuffle_ps(a, b, m)
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm_addsub_ps(_mm_mul_ps(a, b), c)      // SSE3
#define _rk_mm_setzero_si()          _mm_setzero_si128()
#define _rk_mm_unpacklo_epi16(a, b)  _mm_unpacklo_epi16(a, b)                // SSE2
#define _rk_mm_unpackhi_epi16(a, b)  _mm_unpackhi_epi16(a, b)                // SSE2
#define _rk_mm_cvtepi16_epi32(a)     _mm_cvtepi16_epi32(a)                   // SSE4.1
#define _rk_mm_cvtepi32_pf(a)        _mm_cvtepi32_ps(a)                      // SSE2
#define _rk_mm_sqrt_pf(a)            _mm_sqrt_ps(a)
#if defined(_mm_mul_ps)
#define _rk_mm_log10_pf(a)           _mm_log10_ps(a)
#endif

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
void RKSIMD_zscl (RKIQZ *src, const float f, RKIQZ *dst, const int n);
void RKSIMD_izscl(RKIQZ *srcdst, const float f, const int n);
void RKSIMD_zabs(RKIQZ *src, float *dst, const int n);
void RKSIMD_iymul(RKComplex *src, RKComplex *dst, const int n);
void RKSIMD_iymul_reg(RKComplex *src, RKComplex *dst, const int n);
void RKSIMD_ssadd(float *src, const float f, float *dst, const int n);
void RKSIMD_iyscl(RKComplex *src, const float s, const int n);

void RKSIMD_IQZ2Complex(RKIQZ *src, RKComplex *dst, const int n);
void RKSIMD_Complex2IQZ(RKComplex *src, RKIQZ *dst, const int n);
void RKSIMD_Int2Complex(RKInt16C *src, RKComplex *dst, const int n);
void RKSIMD_Int2Complex_reg(RKInt16C *src, RKComplex *dst, const int n);

void RKSIMD_subc(RKFloat *src, const RKFloat f, RKFloat *dst, const int n);

#endif
