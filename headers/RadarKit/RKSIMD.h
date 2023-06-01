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

// CONVENTION: 0,    1,   2,    3, ...
//             even, odd, even, odd, ...

typedef __m512 RKVec;
#define _rk_mm_set1(a)               _mm512_set1_ps(a)
#define _rk_mm_load(a)               _mm512_load_ps(a)
#define _rk_mm_add(a, b)             _mm512_add_ps(a, b)
#define _rk_mm_sub(a, b)             _mm512_sub_ps(a, b)
#define _rk_mm_mul(a, b)             _mm512_mul_ps(a, b)
#define _rk_mm_div(a, b)             _mm512_div_ps(a, b)
#define _rk_mm_min(a, b)             _mm512_min_ps(a, b)
#define _rk_mm_max(a, b)             _mm512_max_ps(a, b)
#define _rk_mm_dup_odd(a)            _mm512_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 1, 1))              //
#define _rk_mm_dup_even(a)           _mm512_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 0, 0))              //
#define _rk_mm_flip_pair(a)          _mm512_shuffle_ps(a, a, _MM_SHUFFLE(2, 3, 0, 1))              //
#define _rk_mm_movehdup(a)           _mm512_movehdup_ps(a)
#define _rk_mm_moveldup(a)           _mm512_moveldup_ps(a)
#if defined(__FMA__)
#define _rk_mm_muladd(a, b, c)       _mm512_fmadd_ps(a, b, c)
#define _rk_mm_muladdsub(a, b, c)    _mm512_fmaddsub_ps(a, b, c)
#define _rk_mm_mulsubadd(a, b, c)    _mm512_fmsubadd_ps(a, b, c)
#endif
#define _rk_mm_sqrt(a)               _mm512_sqrt_ps(a)
#define _rk_mm_rcp(a)                _mm512_rcp_ps(a)

#elif defined(__AVX__)

typedef __m256 RKVec;
#define _rk_mm_set1(a)               _mm256_set1_ps(a)
#define _rk_mm_load(a)               _mm256_load_ps(a)
#define _rk_mm_add(a, b)             _mm256_add_ps(a, b)
#define _rk_mm_sub(a, b)             _mm256_sub_ps(a, b)
#define _rk_mm_mul(a, b)             _mm256_mul_ps(a, b)
#define _rk_mm_div(a, b)             _mm256_div_ps(a, b)
#define _rk_mm_min(a, b)             _mm256_min_ps(a, b)
#define _rk_mm_max(a, b)             _mm256_max_ps(a, b)
#if defined(__builtin_shufflevector)                                                               // macOS, x86_64 AVX, clang
#define _rk_mm_dup_odd(a)            __builtin_shufflevector(a, a, 1, 1, 3, 3, 5, 5, 7, 7)         //
#define _rk_mm_dup_even(a)           __builtin_shufflevector(a, a, 0, 0, 2, 2, 4, 4, 6, 6)         //
#define _rk_mm_flip_pair(a)          __builtin_shufflevector(a, a, 1, 0, 3, 2, 5, 4, 7, 6)         //
#else                                                                                              // Ubuntu/Others, x86_64, CC
#define _rk_mm_dup_odd(a)            _mm256_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 1, 1))             //
#define _rk_mm_dup_even(a)           _mm256_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 0, 0))             //
#define _rk_mm_flip_pair(a)          _mm256_shuffle_ps(a, a, _MM_SHUFFLE(2, 3, 0, 1))             //
#endif
#define _rk_mm_movehdup(a)           _mm256_movehdup_ps(a)                                         // Replaced by _rk_mm_dup_odd
#define _rk_mm_moveldup(a)           _mm256_moveldup_ps(a)                                         // Replaced by _rk_mm_dup_even
#if defined(__FMA__)
#define _rk_mm_muladd(a, b, c)       _mm256_fmadd_ps(a, b, c)
#define _rk_mm_muladdsub(a, b, c)    _mm256_fmaddsub_ps(a, b, c)
#define _rk_mm_mulsubadd(a, b, c)    _mm256_fmsubadd_ps(a, b, c)
#endif
#define _rk_mm_sqrt(a)               _mm256_sqrt_ps(a)
#define _rk_mm_rcp(a)                _mm256_rcp_ps(a)

#elif defined(_EXPLICIT_INTRINSIC) || defined(__x86_64__)

typedef __m128 RKVec;
#define _rk_mm_set1(a)               _mm_set1_ps(a)
#define _rk_mm_load(a)               _mm_load_ps(a)
#define _rk_mm_add(a, b)             _mm_add_ps(a, b)
#define _rk_mm_sub(a, b)             _mm_sub_ps(a, b)
#define _rk_mm_mul(a, b)             _mm_mul_ps(a, b)
#define _rk_mm_div(a, b)             _mm_div_ps(a, b)
#define _rk_mm_min(a, b)             _mm_min_ps(a, b)
#define _rk_mm_max(a, b)             _mm_max_ps(a, b)
#if defined(__builtin_shufflevector)                                                               // macOS, x86_64 SSE, clang
#define _rk_mm_dup_odd(a)            __builtin_shufflevector(a, a, 1, 1, 3, 3)                     //
#define _rk_mm_dup_even(a)           __builtin_shufflevector(a, a, 0, 0, 2, 2)                     //
#define _rk_mm_flip_pair(a)          __builtin_shufflevector(a, a, 1, 0, 3, 2)                     //
#else                                                                                              // Ubuntu/Others, x86_64, CC
#define _rk_mm_dup_odd(a)            _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 1, 1))                 //
#define _rk_mm_dup_even(a)           _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 2, 0, 0))                 //
#define _rk_mm_flip_pair(a)          _mm_shuffle_ps(a, a, _MM_SHUFFLE(2, 3, 0, 1))                 //
#endif
#define _rk_mm_movehdup(a)           _mm_movehdup_ps(a)                                            // Replaced by _rk_mm_dup_odd
#define _rk_mm_moveldup(a)           _mm_moveldup_ps(a)                                            // Replaced by _rk_mm_dup_even
#if defined(__FMA__)
#define _rk_mm_muladd(a, b, c)       _mm_fmadd_ps(a, b, c)
#define _rk_mm_muladdsub(a, b, c)    _mm_fmaddsub_ps(a, b, c)
#define _rk_mm_mulsubadd(a, b, c)    _mm_fmsubadd_ps(a, b, c)
#endif
#define _rk_mm_sqrt(a)               _mm_sqrt_ps(a)
#define _rk_mm_rcp(a)                _mm_rcp_ps(a)

#elif defined(__ARM_NEON__)

typedef float32x4_t RKVec;
#define _rk_mm_set1(a)               vld1q_dup_f32((float *)&a)
#define _rk_mm_load(a)               vld1q_f32(a)
#define _rk_mm_add(a, b)             vaddq_f32(a, b)
#define _rk_mm_sub(a, b)             vsubq_f32(a, b)
#define _rk_mm_mul(a, b)             vmulq_f32(a, b)
#define _rk_mm_div(a, b)             vdivq_f32(a, b)
#define _rk_mm_min(a, b)             vminq_f32(a, b)
#define _rk_mm_max(a, b)             vmaxq_f32(a, b)
#define _rk_mm_dup_odd(a)            __builtin_shufflevector(a, a, 1, 1, 3, 3)                     // macOS, M1, clang
#define _rk_mm_dup_even(a)           __builtin_shufflevector(a, a, 0, 0, 2, 2)                     //
#define _rk_mm_flip_pair(a)          __builtin_shufflevector(a, a, 1, 0, 3, 2)                     //
#define _rk_mm_sqrt(a)               vsqrtq_f32(a)
#define _rk_mm_rcp(a)                vrecpeq_f32(a)

#endif

size_t RKSIMD_size(void);

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
void RKSIMD_iyconj(RKComplex *src, const int n);
void RKSIMD_ssadd(float *src, const float f, float *dst, const int n);
void RKSIMD_iyscl(RKComplex *src, const float s, const int n);

void RKSIMD_IQZ2Complex(RKIQZ *src, RKComplex *dst, const int n);
void RKSIMD_Complex2IQZ(RKComplex *src, RKIQZ *dst, const int n);
void RKSIMD_Int2Complex(RKInt16C *src, RKComplex *dst, const int n);

void RKSIMD_subc(RKFloat *src, const RKFloat f, RKFloat *dst, const int n);
void RKSIMD_clamp(RKFloat *src, const RKFloat min, const RKFloat max, const int n);

void RKSIMD_izrmrm(RKIQZ *src, RKFloat *dst, RKFloat *x, RKFloat *y, RKFloat u, const int n);

#endif
