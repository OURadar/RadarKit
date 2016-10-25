//
//  RKSIMD.c
//  RadarKit
//
//  This library of functions is incomplete.
//  If RKFloat is double, more work will be needed.
//
//  Created by Boon Leng Cheong on 10/18/16.
//
//

#include <RadarKit/RKSIMD.h>

#if defined(__AVX512F__)
typedef __m512 RKVec;
typedef __m256 RKVecCvt;
#define _rk_mm_add_pf(a, b)          _mm512_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm512_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm512_mul_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm512_set1_ps(a)
#define _rk_mm_movehdup_pf(a)        _mm512_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm512_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm512_shuffle_ps(a, b, m)
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm512_fmaddsub_ps(a, b, c)
#define _rk_mm_cvtepi16_epi32(a)     _mm512_cvtepi16_epi32(a)
#define _rk_mm_cvtepi32_pf(a)        _mm512_cvtepi32_ps(a)
#elif defined(__AVX__)
typedef __m256 RKVec;
typedef __m128 RKVecCvt;
#define _rk_mm_add_pf(a, b)          _mm256_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm256_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm256_mul_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm256_set1_ps(a)
#define _rk_mm_movehdup_pf(a)        _mm256_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm256_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm256_shuffle_ps(a, b, m)
//#define _rk_mm_fmaddsub_pf(a, b, c)  _mm256_fmaddsub_ps(a, b, c)
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm256_addsub_ps(_mm256_mul_ps(a, b), c)
#define _rk_mm_cvtepi16_epi32(a)     _mm256_cvtepi16_epi32(a)
#define _rk_mm_cvtepi32_pf(a)        _mm256_cvtepi32_ps(a)
#else
typedef __m128 RKVec;
//typedef __m128 RKVecCvt;
#define _rk_mm_add_pf(a, b)          _mm_add_ps(a, b)
#define _rk_mm_sub_pf(a, b)          _mm_sub_ps(a, b)
#define _rk_mm_mul_pf(a, b)          _mm_mul_ps(a, b)
#define _rk_mm_set1_pf(a)            _mm_set1_ps(a)
#define _rk_mm_movehdup_pf(a)        _mm_movehdup_ps(a)
#define _rk_mm_moveldup_pf(a)        _mm_moveldup_ps(a)
#define _rk_mm_shuffle_pf(a, b, m)   _mm_shuffle_ps(a, b, m)
#define _rk_mm_fmaddsub_pf(a, b, c)  _mm_addsub_ps(_mm_mul_ps(a, b), c)
//#define _rk_mm_cvtepi16_epi32(a)     _mm_cvtepi16_epi32(a)
//#define _rk_mm_cvtepi32_pf(a)        _mm_cvtepi32_ps(a)
#endif

#define OXSTR(x)   x ? "\033[32mo\033[0m" : "\033[31mx\033[0m"

void RKSIMD_show_info(void) {
    #ifdef __AVX512F__
    printf("AVX512F is active.\n");
    #elif __AVX__
    printf("AVX2 256-bit is active.\n");
    #else
    printf("SSE 128-bit is active.\n");
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
        *di++ = _rk_mm_add_pf(*s1i++, *s2i++);
        *dq++ = _rk_mm_add_pf(*s1q++, *s2q++);
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
            *di = _rk_mm_add_pf(_rk_mm_mul_pf(*s1i, *s2i), _rk_mm_mul_pf(*s1q, *s2q)); // I = I1 * I2 + Q1 * Q2
            *dq = _rk_mm_sub_pf(_rk_mm_mul_pf(*s1q, *s2i), _rk_mm_mul_pf(*s1i, *s2q)); // Q = Q1 * I2 - I1 * Q2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    } else {
        for (int k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            *di = _rk_mm_sub_pf(_rk_mm_mul_pf(*s1i, *s2i), _rk_mm_mul_pf(*s1q, *s2q)); // I = I1 * I2 - Q1 * Q2
            *dq = _rk_mm_add_pf(_rk_mm_mul_pf(*s1i, *s2q), _rk_mm_mul_pf(*s1q, *s2i)); // Q = I1 * Q2 + Q1 * I2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    }
    return;
}

// In-place Complex Addition
void RKSIMD_izadd(RKIQZ *src, RKIQZ *dst, const int n) {
    int k;
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *di = _rk_mm_add_pf(*di, *si++);
        *dq = _rk_mm_add_pf(*dq, *sq++);
        di++;
        dq++;
    }
    return;
}

// In-place Complex Multiplication (~50% faster!)
void RKSIMD_izmul(RKIQZ *src, RKIQZ *dst, const int n, const bool c) {
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    RKVec i;
    RKVec q;
    if (c) {
        // Conjugate s2
        for (int k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            i = _rk_mm_add_pf(_rk_mm_mul_pf(*di, *si), _rk_mm_mul_pf(*dq, *sq)); // I = I1 * I2 + Q1 * Q2
            q = _rk_mm_sub_pf(_rk_mm_mul_pf(*dq, *si), _rk_mm_mul_pf(*di, *sq)); // Q = Q1 * I2 - I1 * Q2
            *di++ = i;
            *dq++ = q;
            si++; sq++;
        }
    } else {
        for (int k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            i = _rk_mm_sub_pf(_rk_mm_mul_pf(*di, *si), _rk_mm_mul_pf(*dq, *sq)); // I = I1 * I2 - Q1 * Q2
            q = _rk_mm_add_pf(_rk_mm_mul_pf(*di, *sq), _rk_mm_mul_pf(*dq, *si)); // Q = I1 * Q2 + Q1 * I2
            *di++ = i;
            *dq++ = q;
            si++; sq++;
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
            *di = _rk_mm_add_pf(*di, _rk_mm_add_pf(_rk_mm_mul_pf(*s1i, *s2i), _rk_mm_mul_pf(*s1q, *s2q))); // I += I1 * I2 + Q1 * Q2
            *dq = _rk_mm_add_pf(*dq, _rk_mm_sub_pf(_rk_mm_mul_pf(*s1q, *s2i), _rk_mm_mul_pf(*s1i, *s2q))); // Q += Q1 * I2 - I1 * Q2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    } else {
        for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            *di = _rk_mm_add_pf(*di, _rk_mm_sub_pf(_rk_mm_mul_pf(*s1i, *s2i), _rk_mm_mul_pf(*s1q, *s2q))); // I += I1 * I2 - Q1 * Q2
            *dq = _rk_mm_add_pf(*dq, _rk_mm_add_pf(_rk_mm_mul_pf(*s1i, *s2q), _rk_mm_mul_pf(*s1q, *s2i))); // Q += I1 * Q2 + Q1 * I2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    }
    return;
}

// Multiply by a scale
void RKSIMD_zscl(RKIQZ *src, const float f, RKIQZ *dst, const int n) {
    int k;
    const RKVec fv = _rk_mm_set1_pf(f);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *di++ = _rk_mm_mul_pf(*si++, fv);
        *dq++ = _rk_mm_mul_pf(*sq++, fv);
    }
    return;
}

// Add by a float
void RKSIMD_ssadd(float *src, const RKFloat f, float *dst, const int n) {
    int k;
    const RKVec fv = _rk_mm_set1_pf(f);
    RKVec *s = (RKVec *)src;
    RKVec *d = (RKVec *)dst;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *d++ = _rk_mm_add_pf(*s++, fv);
    }
    return;
}

void RKSIMD_iymul_reg(RKComplex *src, RKComplex *dst, const int n) {
    int k;
    RKFloat fi, fq;
    for (k = 0; k < n; k++) {
        fi = src->i * dst->i - src->q * dst->q;
        fq = src->i * dst->q + src->q * dst->i;
        dst->i = fi;
        dst->q = fq;
        dst++;
        src++;
    }
    return;
}

void RKSIMD_iymul(RKComplex *src, RKComplex *dst, const int n) {
    int k;
    RKVec r, i, x;
    RKVec *s = (RKVec *)src;                                     // [a  b  x  y ]
    RKVec *d = (RKVec *)dst;                                     // [c  d  z  w ]
    for (k = 0; k < (n + 1) * sizeof(RKComplex) / sizeof(RKVec); k++) {
        r = _rk_mm_moveldup_pf(*s);                              // [a  a  x  x ]
        i = _rk_mm_movehdup_pf(*s);                              // [b  b  y  y ]
        x = _rk_mm_shuffle_pf(*d, *d, _MM_SHUFFLE(2, 3, 0, 1));  // [d  c  w  z ]
        i = _rk_mm_mul_pf(i, x);                                 // [bd bc yw yz]
        *d = _rk_mm_fmaddsub_pf(r, *d, i);                       // [a a x x] * [c d z w] -/+ [bd bc yw yz] = [ac-bd ad+bc xz-yw xw+yz]
        s++;
        d++;
    }
    return;
}

void RKSIMD_iyscl(RKComplex *src, const RKFloat m, const int n) {
    int k;
    RKVec *s = (RKVec *)src;
    RKVec mv = _rk_mm_set1_pf(m);
    for (k = 0; k < (n + 1) * sizeof(RKComplex) / sizeof(RKVec); k++) {
        *s++ *= mv;
    }
    return;
}

void RKSIMD_IQZ2Complex(RKIQZ *src, RKComplex *dst, const int n) {
    RKFloat *si = &src->i[0];
    RKFloat *sq = &src->q[0];
    RKFloat *d = &dst->i;
    for (int k = 0; k < n; k++) {
        *d++ = *si++;
        *d++ = *sq++;
    }
    return;
}

void RKSIMD_Complex2IQZ(RKComplex *src, RKIQZ *dst, const int n) {
    RKFloat *s = &src[0].i;
    RKFloat *di = &dst->i[0];
    RKFloat *dq = &dst->q[0];
    for (int k = 0; k < n; k++) {
        *di++ = *s++;
        *dq++ = *s++;
    }
    return;
}

void RKSIMD_Int2Complex(RKInt16 *src, RKComplex *dst, const int n) {
#if defined(__AVX512F__) || defined(__AVX__)
    RKVec *s = (RKVecCvt *)src;
    RKVec *d = (RKVec *)dst;
    for (int k = 0; k < (n + 1) * sizeof(RKComplex) / sizeof(RKVec); k++) {
        *d++ = _rk_mm_cvtepi32_pf(_rk_mm_cvtepi16_epi32(*s++));
    }
#else
    return RKSIMD_Int2Complex_reg(src, dst, n);
#endif
}

void RKSIMD_Int2Complex_reg(RKInt16 *src, RKComplex *dst, const int n) {
    for (int i = 0; i < n; i++) {
        dst[i].i = (RKFloat)src[i].i;
        dst[i].q = (RKFloat)src[i].q;
    }
    return;
}

#define RKSIMD_TEST_DESC_FORMAT        "%65s"
#define RKSIMD_TEST_RESULT(str, res)   printf(RKSIMD_TEST_DESC_FORMAT " : %s.\033[0m\n", str, res ? "\033[32msuccessful" : "\033[31mfailed");

void RKSIMDDemo(const RKSIMDDemoFlag flag) {
    RKSIMD_show_info();

    int i;
    RKIQZ *src, *dst, *cpy;
    posix_memalign((void **)&src, RKSIMDAlignSize, sizeof(RKIQZ));
    posix_memalign((void **)&dst, RKSIMDAlignSize, sizeof(RKIQZ));
    posix_memalign((void **)&cpy, RKSIMDAlignSize, sizeof(RKIQZ));
    memset(dst, 0, sizeof(RKIQZ));
    const int n = 32;

    const RKFloat tiny = 1.0e-3f;
    bool good;
    bool all_good = true;

    //

    for ( i = 0; i < n; i++) {
        src->i[i] = (RKFloat)i;
        src->q[i] = (RKFloat)-i;
    }

    RKSIMD_zcpy(src, dst, n);

    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        good = src->i[i] == dst->i[i] && src->q[i] == dst->q[i];
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("src[%2d] = %9.2f%+9.2fi   dst[%2d] = %9.2f%+9.2fi\n", i, src->i[i], src->q[i], i, dst->i[i], dst->q[i]);
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Copy -  zcpy", all_good);

    //

    RKSIMD_zscl(src, 3.0f, dst, n);

    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 3-3i, 6-6i, 9-9i, ...
        good = fabsf(dst->i[i] - 3.0f * i) < tiny && fabs(dst->q[i] + 3.0f * i) < tiny;
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("%9.2f%+9.2fi x 3.0 -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Scaling by a Float -  zscl", all_good);
    
    //

    RKSIMD_zadd(src, src, dst, n);

    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Addition -  zadd", all_good);

    //

    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izadd(src, dst, n);

    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (int i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("In-place Complex Vector Addition - izadd", all_good);

    //

    RKSIMD_zmul(src, src, dst, n, false);

    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Multiplication -  zmul", all_good);

    //

    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izmul(src, dst, n, false);

    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("In-place Complex Vector Multiplication - izmul", all_good);

    //

    RKComplex *cs = (RKComplex *)src;
    RKComplex *cd = (RKComplex *)dst;
    RKComplex *cc = (RKComplex *)cpy;

    // Populate some numbers
    for (i = 0; i < n; i++) {
        cs[i].i = (RKFloat)i;
        cs[i].q = (RKFloat)(-i);
        cd[i].i = (RKFloat)(i + 1);
        cd[i].q = (RKFloat)(-i);
    }

    memcpy(cc, cd, n * sizeof(RKComplex));
    RKSIMD_iymul(cs, cd, n);

    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be  ... 0, 1-3i, 2-10i, 3-21i, ...
        good = fabsf(cd[i].i - (RKFloat)i) < tiny && fabsf(cd[i].q - (RKFloat)(-2 * i * i - i)) < tiny;
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("%+9.2f%+9.2f * %+9.2f%+9.2f = %+9.2f%+9.2f  %s\n", cs[i].i, cs[i].q, cc[i].i, cc[i].q, cd[i].i, cd[i].q, OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("In-place Deinterleaved Complex Vector Multiplication - iymul", all_good);

    //

    for (i = 0; i < n; i++) {
        cc[i].i = (RKFloat)i;
        cc[i].q = (RKFloat)(-i);
    }
    RKSIMD_Complex2IQZ(cc, src, n);
    for (i = 0; i < n; i++) {
        cc[i].i = (RKFloat)(i + 1);
        cc[i].q = (RKFloat)(-i);
    }
    RKSIMD_Complex2IQZ(cc, dst, n);
    RKSIMD_izmul(src, dst, n, false);
    RKSIMD_IQZ2Complex(dst, cc, n);
    if (flag & RKSIMDDemoFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be  ... 0, 1-3i, 2-10i, 3-21i, ...
        good = fabsf(cc[i].i - (RKFloat)i) < tiny && fabsf(cc[i].q - (RKFloat)(-2 * i * i - i)) < tiny;
        if (flag & RKSIMDDemoFlagShowNumbers) {
            printf("%+9.2f%+9.2f * %+9.2f%+9.2f = %+9.2f%+9.2f  %s\n", src->i[i], src->q[i], cpy->i[i], cpy->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Deinterleave, Multiply Using iymul, and Interleave", all_good);

    if (flag > 1) {
        printf("\n==== Performance test ====\n\n");

        int k;
        const int m = 100000;
        struct timeval t1, t2;

        gettimeofday(&t1, NULL);
        for (k = 0; k < m; k++) {
            RKSIMD_zmul(src, src, dst, RKGateCount, false);
        }
        gettimeofday(&t2, NULL);
        printf("Regular SIMD multiplication time for %dK loops = %.3fs\n", m / 1000, RKTimevalDiff(t2, t1));

        gettimeofday(&t1, NULL);
        for (k = 0; k < m; k++) {
            RKSIMD_izmul(src, dst, RKGateCount, false);
        }
        gettimeofday(&t2, NULL);
        printf("In-place SIMD multiplication time for %dK loops = %.3fs\n", m / 1000, RKTimevalDiff(t2, t1));

        printf("Vectorized Complex Multiplication (%dK loops):\n", m / 1000);
        gettimeofday(&t1, NULL);
        for (k = 0; k < m; k++) {
            RKSIMD_iymul_reg(cs, cd, RKGateCount);
        }
        gettimeofday(&t2, NULL);
        printf("              -Os: %.3fs (compiler optimized)\n", RKTimevalDiff(t2, t1));

        gettimeofday(&t1, NULL);
        for (k = 0; k < m; k++) {
            RKSIMD_iymul(cs, cd, RKGateCount);
        }
        gettimeofday(&t2, NULL);
        printf("            iymul: %.3fs (Normal interleaved I/Q)\n", RKTimevalDiff(t2, t1));

        gettimeofday(&t1, NULL);
        for (k = 0; k < m; k++) {
            RKSIMD_izmul((RKIQZ *)src, (RKIQZ *)dst, RKGateCount, false);
        }
        gettimeofday(&t2, NULL);
        printf("            izmul: %.3fs (Deinterleaved I/Q)\n", RKTimevalDiff(t2, t1));

        gettimeofday(&t1, NULL);
        for (k = 0; k < m; k++) {
            RKSIMD_Complex2IQZ(cc, src, RKGateCount);
            RKSIMD_izmul((RKIQZ *)src, (RKIQZ *)dst, RKGateCount, false);
            RKSIMD_IQZ2Complex(dst, cc, RKGateCount);
        }
        gettimeofday(&t2, NULL);
        printf("    E + izmul + D: %.3fs (D, Multiply, I)\n", RKTimevalDiff(t2, t1));
        printf("\n==========================\n");
}

    free(src);
    free(dst);
}
