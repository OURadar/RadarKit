//
//  RKSIMD.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/18/16.
//
//

#include <RadarKit/RKSIMD.h>

#ifdef __AVX2__
typedef __m512 RKVec;
#define _rk_mm_add_ps(a, b)  _mm512_add_ps(a, b)
#define _rk_mm_sub_ps(a, b)  _mm512_sub_ps(a, b)
#define _rk_mm_mul_ps(a, b)  _mm512_mul_ps(a, b)
#define _rk_mm_set1_ps(a)    _mm512_set1_ps(a)
#elif __AVX__
typedef __m256 RKVec;
#define _rk_mm_add_ps(a, b)  _mm256_add_ps(a, b)
#define _rk_mm_sub_ps(a, b)  _mm256_add_ps(a, b)
#define _rk_mm_mul_ps(a, b)  _mm256_mul_ps(a, b)
#define _rk_mm_set1_ps(a)    _mm256_set1_ps(a)
#else
typedef __m128 RKVec;
#define _rk_mm_add_ps(a, b)  _mm_add_ps(a, b)
#define _rk_mm_sub_ps(a, b)  _mm_sub_ps(a, b)
#define _rk_mm_mul_ps(a, b)  _mm_mul_ps(a, b)
#define _rk_mm_set1_ps(a)    _mm_set1_ps(a)
#endif

void RKSIMD_show_info(void) {
    #ifdef __AVX2__
    printf("AVX2 is active.\n");
    #elif __AVX__
    printf("AVX is active.\n");
    #else
    printf("SSE is active.\n");
    #endif
    printf("sizeof(RKVec) = %zu (%zu floats)\n", sizeof(RKVec), sizeof(RKVec) / 4);
    return;
}

//
// Complex operations
//

// Complex copy
void RKSIMD_zcpy(RKIQZ *src, RKIQZ *dst, const int n) {
    int k;
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *di++ = *si++;
        *dq++ = *sq++;
    }
    return;
}

// Complex Addition
void RKSIMD_zadd(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n) {
    int k;
    RKVec *s1i = (RKVec *)s1->i;
    RKVec *s1q = (RKVec *)s1->q;
    RKVec *s2i = (RKVec *)s2->i;
    RKVec *s2q = (RKVec *)s2->q;
    RKVec *di  = (RKVec *)dst->i;
    RKVec *dq  = (RKVec *)dst->q;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *di++ = _rk_mm_add_ps(*s1i++, *s2i++);
        *dq++ = _rk_mm_add_ps(*s1q++, *s2q++);
    }
    return;
}

// Complex Multiplication
void RKSIMD_zmul(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n, const bool c) {
    RKVec *s1i = (RKVec *)s1->i;
    RKVec *s1q = (RKVec *)s1->q;
    RKVec *s2i = (RKVec *)s2->i;
    RKVec *s2q = (RKVec *)s2->q;
    RKVec *di  = (RKVec *)dst->i;
    RKVec *dq  = (RKVec *)dst->q;
    if (c) {
        // Conjugate s2
        for (int k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            *di = _rk_mm_add_ps(_rk_mm_mul_ps(*s1i, *s2i), _rk_mm_mul_ps(*s1q, *s2q)); // I = I1 * I2 + Q1 * Q2
            *dq = _rk_mm_sub_ps(_rk_mm_mul_ps(*s1q, *s2i), _rk_mm_mul_ps(*s1i, *s2q)); // Q = Q1 * I2 - I1 * Q2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    } else {
        for (int k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            *di = _rk_mm_sub_ps(_rk_mm_mul_ps(*s1i, *s2i), _rk_mm_mul_ps(*s1q, *s2q)); // I = I1 * I2 - Q1 * Q2
            *dq = _rk_mm_add_ps(_rk_mm_mul_ps(*s1i, *s2q), _rk_mm_mul_ps(*s1q, *s2i)); // Q = I1 * Q2 + Q1 * I2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    }
    return;
}

// Accumulate multiply add
void RKSIMD_zcma(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n, const bool c) {
    int k;
    RKVec *s1i = (RKVec *)s1->i;
    RKVec *s1q = (RKVec *)s1->q;
    RKVec *s2i = (RKVec *)s2->i;
    RKVec *s2q = (RKVec *)s2->q;
    RKVec *di  = (RKVec *)dst->i;
    RKVec *dq  = (RKVec *)dst->q;
    if (c) {
        // Conjugate s2
        for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            *di = _rk_mm_add_ps(*di, _rk_mm_add_ps(_rk_mm_mul_ps(*s1i, *s2i), _rk_mm_mul_ps(*s1q, *s2q))); // I += I1 * I2 + Q1 * Q2
            *dq = _rk_mm_add_ps(*dq, _rk_mm_sub_ps(_rk_mm_mul_ps(*s1q, *s2i), _rk_mm_mul_ps(*s1i, *s2q))); // Q += Q1 * I2 - I1 * Q2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    } else {
        for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            *di = _rk_mm_add_ps(*di, _rk_mm_sub_ps(_rk_mm_mul_ps(*s1i, *s2i), _rk_mm_mul_ps(*s1q, *s2q))); // I += I1 * I2 - Q1 * Q2
            *dq = _rk_mm_add_ps(*dq, _rk_mm_add_ps(_rk_mm_mul_ps(*s1i, *s2q), _rk_mm_mul_ps(*s1q, *s2i))); // Q += I1 * Q2 + Q1 * I2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    }
    return;
}

// Multiply by a scale
void RKSIMD_zsmul(RKIQZ *src, const float f, RKIQZ *dst, const int n) {
    int k;
    const RKVec fv = _rk_mm_set1_ps(f);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *di++ = _rk_mm_mul_ps(*si++, fv);
        *dq++ = _rk_mm_mul_ps(*sq++, fv);
    }
    return;
}

// Add by a float
void RKSIMD_ssadd(float *src, const float f, float *dst, const int n) {
    int k;
    const RKVec fv = _rk_mm_set1_ps(f);
    RKVec *s = (RKVec *)src;
    RKVec *d = (RKVec *)dst;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *d++ = _rk_mm_add_ps(*s++, fv);
    }
    return;
}
