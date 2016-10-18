//
//  RKSIMD.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/18/16.
//
//

#include <RadarKit/RKSIMD.h>

#ifdef __AVX512F__
typedef __m512 RKVec;
#define _rk_mm_add_ps(a, b)  _mm512_add_ps(a, b)
#define _rk_mm_sub_ps(a, b)  _mm512_sub_ps(a, b)
#define _rk_mm_mul_ps(a, b)  _mm512_mul_ps(a, b)
#define _rk_mm_set1_ps(a)    _mm512_set1_ps(a)
#elif __AVX__
typedef __m256 RKVec;
#define _rk_mm_add_ps(a, b)  _mm256_add_ps(a, b)
#define _rk_mm_sub_ps(a, b)  _mm256_sub_ps(a, b)
#define _rk_mm_mul_ps(a, b)  _mm256_mul_ps(a, b)
#define _rk_mm_set1_ps(a)    _mm256_set1_ps(a)
#else
typedef __m128 RKVec;
#define _rk_mm_add_ps(a, b)  _mm_add_ps(a, b)
#define _rk_mm_sub_ps(a, b)  _mm_sub_ps(a, b)
#define _rk_mm_mul_ps(a, b)  _mm_mul_ps(a, b)
#define _rk_mm_set1_ps(a)    _mm_set1_ps(a)
#endif

#define OXSTR(x)   x ? "\033[32mo\033[0m" : "\033[31mx\033[0m"

void RKSIMD_show_info(void) {
    #ifdef __AVX512F__
    printf("AVX512F is active.\n");
    #elif __AVX2__
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

// In-place Complex Addition
void RKSIMD_izadd(RKIQZ *src, RKIQZ *dst, const int n) {
    int k;
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
        *di = _rk_mm_add_ps(*di, *si++);
        *dq = _rk_mm_add_ps(*dq, *sq++);
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
            i = _rk_mm_add_ps(_rk_mm_mul_ps(*di, *si), _rk_mm_mul_ps(*dq, *sq)); // I = I1 * I2 + Q1 * Q2
            q = _rk_mm_sub_ps(_rk_mm_mul_ps(*dq, *si), _rk_mm_mul_ps(*di, *sq)); // Q = Q1 * I2 - I1 * Q2
            *di++ = i;
            *dq++ = q;
            si++; sq++;
        }
    } else {
        for (int k = 0; k < (n + 1) * sizeof(RKFloat) / sizeof(RKVec); k++) {
            i = _rk_mm_sub_ps(_rk_mm_mul_ps(*di, *si), _rk_mm_mul_ps(*dq, *sq)); // I = I1 * I2 - Q1 * Q2
            q = _rk_mm_add_ps(_rk_mm_mul_ps(*di, *sq), _rk_mm_mul_ps(*dq, *si)); // Q = I1 * Q2 + Q1 * I2
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

void RKSIMD_iymul(RKComplex *src, RKComplex *dst, const int n) {
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
//    __m128 qH, qL, s2s;
//    __m128 *s = (__m128 *)src;
//    __m128 *d = (__m128 *)dst;
//    for (k = 0; k < (n + 1) / 2; k++) {
//        qH = _mm_movehdup_ps(*d);
//        qL = _mm_movehdup_ps(*d);
//        s2s = _mm_shuffle_ps(*s, *s, _MM_SHUFFLE(2, 3, 0, 1));
//        qH = _mm_mul_ps(qH, s2s);
//        qL = _mm_mul_ps(qL, *s++);
//        *d++ = _mm_addsub_ps(qL, qH);
//    }
    return;
}

void RKSIMDDemo(const int show) {
    RKSIMD_show_info();

    int i;
    RKIQZ *src, *dst;
    posix_memalign((void *)&src, RKSIMDAlignSize, sizeof(RKIQZ));
    posix_memalign((void *)&dst, RKSIMDAlignSize, sizeof(RKIQZ));
    memset(dst, 0, sizeof(RKIQZ));
    const int n = 32;

    const RKFloat tiny = 1.0e-3f;
    bool good;
    bool all_good = true;

    for ( i = 0; i < n; i++) {
        src->i[i] = (RKFloat)i;
        src->q[i] = (RKFloat)-i;
    }

    RKSIMD_zcpy(src, dst, n);

    if (show) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        good = src->i[i] == dst->i[i] && src->q[i] == dst->q[i];
        if (show) {
            printf("src[%2d] = %9.2f%+9.2fi   dst[%2d] = %9.2f%+9.2fi\n", i, src->i[i], src->q[i], i, dst->i[i], dst->q[i]);
        }
        all_good &= good;
    }
    if (all_good) {
        printf("Vector Copy \033[32msuccessful\033[0m.\n");
    } else {
        printf("Vector Copy \033[31mfailed\033[0m.\n");
    }

    RKSIMD_zadd(src, src, dst, n);

    if (show) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (show) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    if (all_good) {
        printf("Vector Addition \033[32msuccessful\033[0m.\n");
    } else {
        printf("Vector Addition \033[31mfailed\033[0m.\n");
    }

    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izadd(src, dst, n);

    if (show) {
        printf("====\n");
    }
    all_good = true;
    for (int i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (show) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    if (all_good) {
        printf("In-place Vector Addition \033[32msuccessful\033[0m.\n");
    } else {
        printf("In-place Vector Addition \033[31mfailed\033[0m.\n");
    }

    RKSIMD_zmul(src, src, dst, n, false);

    if (show) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (show) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    if (all_good) {
        printf("Vector Multiplication \033[32msuccessful\033[0m.\n");
    } else {
        printf("Vector Multiplication \033[31mfailed\033[0m.\n");
    }

    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izmul(src, dst, n, false);

    if (show) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (show) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    if (all_good) {
        printf("In-place Vector Multiplication \033[32msuccessful\033[0m.\n");
    } else {
        printf("In-place Vector Multiplication \033[31mfailed\033[0m.\n");
    }

    RKSIMD_zsmul(src, 3.0f, dst, n);

    if (show) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 3-3i, 6-6i, 9-9i, ...
        good = fabsf(dst->i[i] - 3.0f * i) < tiny && fabs(dst->q[i] + 3.0f * i) < tiny;
        if (show) {
            printf("%9.2f%+9.2fi x 3.0 -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    if (all_good) {
        printf("Vector Scaling \033[32msuccessful\033[0m.\n");
    } else {
        printf("Vector Scaling \033[31mfailed\033[0m.\n");
    }

    if (show > 1) {
        printf("====\n");

        struct timeval t1, t2;
        gettimeofday(&t1, NULL);
        for (int i = 0; i < 1000000; i++) {
            RKSIMD_zmul(src, src, dst, RKGateCount, false);
        }
        gettimeofday(&t2, NULL);
        printf("Regular multiplication time for 1M loops = %.3fs\n", RKTimevalDiff(t2, t1));

        gettimeofday(&t1, NULL);
        for (int i = 0; i < 1000000; i++) {
            RKSIMD_izmul(src, dst, RKGateCount, false);
        }
        gettimeofday(&t2, NULL);
        printf("In-place multiplication time for 1M loops = %.3fs\n", RKTimevalDiff(t2, t1));
        
        printf("====\n");
   }

    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    for (int i = 0; i < 1000000; i++) {
        RKSIMD_iymul((RKComplex *)src, (RKComplex *)dst, RKGateCount);
    }
    gettimeofday(&t2, NULL);
    printf("Regular multiplication time for 1M loops = %.3fs\n", RKTimevalDiff(t2, t1));


    free(src);
    free(dst);
}
