//
//  RKSIMD.h
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/18/16.
//
//

#ifndef __RadarKit_RKSIMD__
#define __RadarKit_RKSIMD__

//#include <mmintrin.h>    // MMX
//#include <xmmintrin.h>   // SSE
//#include <emmintrin.h>   // SSE2
//#include <pmmintrin.h>   // SSE3
//#include <tmmintrin.h>   // SSSE3
//#include <smmintrin.h>   // SSE4.1
//#include <nmmintrin.h>   // SSE4.2
//#include <ammintrin.h>   // SSE4A
//#include <wmmintrin.h>   // AES
//#include <immintrin.h>   // AVX
//#include <zmmintrin.h>   // AVX512

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

#include <RadarKit/RKTypes.h>

void RKSIMD_show_info(void);
void RKSIMD_zcpy(RKIQZ *src, RKIQZ *dst, const int n);
void RKSIMD_zadd(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n);
void RKSIMD_zcma(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n, const bool c);
void RKSIMD_zsmul(RKIQZ *src, const float f, RKIQZ *dst, const int n);
void RKSIMD_ssadd(float *src, const float f, float *dst, const int n);

#endif
