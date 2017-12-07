//
//  RKSIMD.c
//  RadarKit
//
//  This library of functions is currently implemented on single precision, i.e., RKFloat = float
//  It has only been tested on several machines that are SSE, SSE2, SSE3, SSE4.1, AVX, AVX2 capable.
//  The AVX512 intrinsics are my best guess at the time of developement. They have not been tested.
//
//  IMPORTANT: If RKFloat is double, more (a lot more) work will be needed.
//
//  Created by Boon Leng Cheong on 10/18/16.
//
//

#include <RadarKit/RKSIMD.h>

void RKSIMD_show_info(void) {
    printf("SIMD Info:\n==========\n");
    #if defined(__AVX512F__)
    printf("AVX512F is active.\n");
    #elif defined(__AVX__)
    #  if defined(__AVX2__)
    printf("AVX & AVX2 256-bit is active.\n");
    #  else
    printf("AVX 256-bit (partial 128-bit) is active.\n");
    #  endif
    #else
    printf("SSE 128-bit is active.\n");
    #endif
    printf("sizeof(RKVec) = %zu B (%zu RKFloats)\n", sizeof(RKVec), sizeof(RKVec) / sizeof(RKFloat));
    return;
}

void RKSIMD_show_count(const int n) {
    int KF = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    int KC = (n * sizeof(RKComplex) + sizeof(RKVec) - 1) / sizeof(RKVec);
    printf("n = %d   K = %d / %d\n", n, KF, KC);
}

//
// Single operations
//
void RKSIMD_mul(RKFloat *src1, RKFloat *src2, RKFloat *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *s1 = (RKVec *)src1;
    RKVec *s2 = (RKVec *)src2;
    RKVec *d  = (RKVec *)dst;
    for (k = 0; k < K; k++) {
        *d++ = _rk_mm_mul_pf(*s1++, *s2++);
    }
    return;
}

//
// Complex operations
//

// Complex copy
void RKSIMD_zcpy(RKIQZ *src, RKIQZ *dst, const int n) {
    int k, N = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < N; k++) {
        *di++ = *si++;
        *dq++ = *sq++;
    }
    return;
}

// Complex Addition
void RKSIMD_zadd(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *s1i = (RKVec *)s1->i;
    RKVec *s1q = (RKVec *)s1->q;
    RKVec *s2i = (RKVec *)s2->i;
    RKVec *s2q = (RKVec *)s2->q;
    RKVec *di  = (RKVec *)dst->i;
    RKVec *dq  = (RKVec *)dst->q;
    for (k = 0; k < K; k++) {
        *di++ = _rk_mm_add_pf(*s1i++, *s2i++);
        *dq++ = _rk_mm_add_pf(*s1q++, *s2q++);
    }
    return;
}

// Complex Subtration
void RKSIMD_zsub(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *s1i = (RKVec *)s1->i;
    RKVec *s1q = (RKVec *)s1->q;
    RKVec *s2i = (RKVec *)s2->i;
    RKVec *s2q = (RKVec *)s2->q;
    RKVec *di  = (RKVec *)dst->i;
    RKVec *dq  = (RKVec *)dst->q;
    for (k = 0; k < K; k++) {
        *di++ = _rk_mm_sub_pf(*s1i++, *s2i++);
        *dq++ = _rk_mm_sub_pf(*s1q++, *s2q++);
    }
    return;
}

// Complex Multiplication
void RKSIMD_zmul(RKIQZ *s1, RKIQZ *s2, RKIQZ *dst, const int n, const bool c) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *s1i = (RKVec *)s1->i;
    RKVec *s1q = (RKVec *)s1->q;
    RKVec *s2i = (RKVec *)s2->i;
    RKVec *s2q = (RKVec *)s2->q;
    RKVec *di  = (RKVec *)dst->i;
    RKVec *dq  = (RKVec *)dst->q;
    if (c) {
        // Conjugate s2
        for (k = 0; k < K; k++) {
            *di++ = _rk_mm_add_pf(_rk_mm_mul_pf(*s1i, *s2i), _rk_mm_mul_pf(*s1q, *s2q)); // I = I1 * I2 + Q1 * Q2
            *dq++ = _rk_mm_sub_pf(_rk_mm_mul_pf(*s1q, *s2i), _rk_mm_mul_pf(*s1i, *s2q)); // Q = Q1 * I2 - I1 * Q2
            s1i++; s1q++;
            s2i++; s2q++;
        }
    } else {
        for (k = 0; k < K; k++) {
            *di++ = _rk_mm_sub_pf(_rk_mm_mul_pf(*s1i, *s2i), _rk_mm_mul_pf(*s1q, *s2q)); // I = I1 * I2 - Q1 * Q2
            *dq++ = _rk_mm_add_pf(_rk_mm_mul_pf(*s1i, *s2q), _rk_mm_mul_pf(*s1q, *s2i)); // Q = I1 * Q2 + Q1 * I2
            s1i++; s1q++;
            s2i++; s2q++;
        }
    }
    return;
}

// Complex Self Multiplication
void RKSIMD_zsmul(RKIQZ *src, RKIQZ *dst, const int n, const bool c) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    if (c) {
        // Conjugate s2
        for (k = 0; k < K; k++) {
            *di++ = _rk_mm_add_pf(_rk_mm_mul_pf(*si, *si), _rk_mm_mul_pf(*sq, *sq)); // I = I1 * I2 + Q1 * Q2
            *dq++ = _rk_mm_sub_pf(_rk_mm_mul_pf(*sq, *si), _rk_mm_mul_pf(*si, *sq)); // Q = Q1 * I2 - I1 * Q2
            si++; sq++;
        }
    } else {
        for (k = 0; k < K; k++) {
            *di++ = _rk_mm_sub_pf(_rk_mm_mul_pf(*si, *si), _rk_mm_mul_pf(*sq, *sq)); // I = I1 * I2 - Q1 * Q2
            *dq++ = _rk_mm_add_pf(_rk_mm_mul_pf(*si, *sq), _rk_mm_mul_pf(*sq, *si)); // Q = I1 * Q2 + Q1 * I2
            si++; sq++;
        }
    }
    return;
}

// In-place Complex Addition
void RKSIMD_izadd(RKIQZ *src, RKIQZ *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < K; k++) {
        *di = _rk_mm_add_pf(*di, *si++);
        *dq = _rk_mm_add_pf(*dq, *sq++);
        di++;
        dq++;
    }
    return;
}

// In-place Complex Subtraction
void RKSIMD_izsub(RKIQZ *src, RKIQZ *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < K; k++) {
        *di = _rk_mm_sub_pf(*di, *si++);
        *dq = _rk_mm_sub_pf(*dq, *sq++);
        di++;
        dq++;
    }
    return;
}

// In-place Complex Multiplication (~50% faster!)
void RKSIMD_izmul(RKIQZ *src, RKIQZ *dst, const int n, const bool c) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    RKVec i;
    RKVec q;
    if (c) {
        // Conjugate s2
        for (k = 0; k < K; k++) {
            i = _rk_mm_add_pf(_rk_mm_mul_pf(*di, *si), _rk_mm_mul_pf(*dq, *sq)); // I = I1 * I2 + Q1 * Q2
            q = _rk_mm_sub_pf(_rk_mm_mul_pf(*dq, *si), _rk_mm_mul_pf(*di, *sq)); // Q = Q1 * I2 - I1 * Q2
            *di++ = i;
            *dq++ = q;
            si++; sq++;
        }
    } else {
        for (k = 0; k < K; k++) {
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
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *s1i = (RKVec *)s1->i;
    RKVec *s1q = (RKVec *)s1->q;
    RKVec *s2i = (RKVec *)s2->i;
    RKVec *s2q = (RKVec *)s2->q;
    RKVec *di  = (RKVec *)dst->i;
    RKVec *dq  = (RKVec *)dst->q;
    if (c) {
        // Conjugate s2
        for (k = 0; k < K; k++) {
            *di = _rk_mm_add_pf(*di, _rk_mm_add_pf(_rk_mm_mul_pf(*s1i, *s2i), _rk_mm_mul_pf(*s1q, *s2q))); // I += I1 * I2 + Q1 * Q2
            *dq = _rk_mm_add_pf(*dq, _rk_mm_sub_pf(_rk_mm_mul_pf(*s1q, *s2i), _rk_mm_mul_pf(*s1i, *s2q))); // Q += Q1 * I2 - I1 * Q2
            s1i++; s1q++;
            s2i++; s2q++;
            di++; dq++;
        }
    } else {
        for (k = 0; k < K; k++) {
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
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    const RKVec fv = _rk_mm_set1_pf(f);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *di = (RKVec *)dst->i;
    RKVec *dq = (RKVec *)dst->q;
    for (k = 0; k < K; k++) {
        *di++ = _rk_mm_mul_pf(*si++, fv);
        *dq++ = _rk_mm_mul_pf(*sq++, fv);
    }
    return;
}

void RKSIMD_izscl(RKIQZ *srcdst, const float f, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    const RKVec fv = _rk_mm_set1_pf(f);
    RKVec *si = (RKVec *)srcdst->i;
    RKVec *sq = (RKVec *)srcdst->q;
    for (k = 0; k < K; k++) {
        *si = _rk_mm_mul_pf(*si, fv);
        *sq = _rk_mm_mul_pf(*sq, fv);
        si++;
        sq++;
    }
    return;
}

// Absolute value of a complex number
void RKSIMD_zabs(RKIQZ *src, float *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *d = (RKVec *)dst;
    for (k = 0; k < K; k++) {
        *d++ = _rk_mm_sqrt_pf(_rk_mm_add_pf(_rk_mm_mul_pf(*si, *si), _rk_mm_mul_pf(*sq, *sq)));
        si++;
        sq++;
    }
}

// Add by a float
void RKSIMD_ssadd(float *src, const RKFloat f, float *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    const RKVec fv = _rk_mm_set1_pf(f);
    RKVec *s = (RKVec *)src;
    RKVec *d = (RKVec *)dst;
    for (k = 0; k < K; k++) {
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

void RKSIMD_yconj(RKComplex *src, const int n) {
    RKFloat *s = (RKFloat *)src;
    s++;
	for (int k = 0; k < n; k++) {
        *s = -*s;
        s += 2;
	}
    // for (int k = 0; k < n; k++) {
    //     src->q = -src->q;
    //     src++;
    // }
	return;
}

void RKSIMD_iymul(RKComplex *src, RKComplex *dst, const int n) {
    int k, K = (n * sizeof(RKComplex) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec r, i, x;
    RKVec *s = (RKVec *)src;                                     // [a  b  x  y ]
    RKVec *d = (RKVec *)dst;                                     // [c  d  z  w ]
    for (k = 0; k < K; k++) {
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

void RKSIMD_iymulc(RKComplex *src, RKComplex *dst, const int n) {
	int k, K = (n * sizeof(RKComplex) + sizeof(RKVec) - 1) / sizeof(RKVec);
	RKVec r, i, x;
	RKVec *s = (RKVec *)src;                                     // [ a  b  x  y ]
	RKVec *d = (RKVec *)dst;                                     // [ c  d  z  w ]
	RKVec c = _rk_mm_set_pf(1.0, -1.0, 1.0, -1.0);
	for (k = 0; k < K; k++) {
		r = _rk_mm_moveldup_pf(*s);                              // [  a   a   x   x ]
		i = _rk_mm_movehdup_pf(*s);                              // [  b   b   y   y ]
		*d = _rk_mm_mul_pf(*d, c);                               // [  c  -d   z  -w ]
		x = _rk_mm_shuffle_pf(*d, *d, _MM_SHUFFLE(2, 3, 0, 1));  // [ -d   c  -w   z ]
		i = _rk_mm_mul_pf(i, x);                                 // [-bd  bc -yw  yz ]
		*d = _rk_mm_fmaddsub_pf(r, *d, i);                       // [a a x x] * [c -d z -w] -/+ [-bd bc -yw yz] = [ac+bd bc-ad xz+yw yz-xw]
		s++;
		d++;
	}
	return;
}

void RKSIMD_iymul2(RKComplex *src, RKComplex *dst, const int n, const bool c) {
	int k, K = (n * sizeof(RKComplex) + sizeof(RKVec) - 1) / sizeof(RKVec);
	RKVec r, i, x;
	RKVec *s = (RKVec *)src;                                         // [a  b  x  y ]
	RKVec *d = (RKVec *)dst;                                         // [c  d  z  w ]
	if (c) {
#if defined(_rk_mm_fmsubadd_pf)
		for (k = 0; k < K; k++) {
			r = _rk_mm_moveldup_pf(*s);                              // [a  a  x  x ]
			i = _rk_mm_movehdup_pf(*s);                              // [b  b  y  y ]
			x = _rk_mm_shuffle_pf(*d, *d, _MM_SHUFFLE(2, 3, 0, 1));  // [d  c  w  z ]
			r = _rk_mm_mul_pf(r, *d);                                // [ac ad xz xw]
			*d = _rk_mm_fmsubadd_pf(i, x, r);                        // [b b y y] * [d c w z] +/- [ac ad xz xw] = [bd+ac bc-ad yw+xz yz-xw]
			s++;
			d++;
		}
#else
		RKComplex *y = dst;
		for (k = 0; k < n; k++) {
			y->q = -y->q;
			y++;
		}
		for (k = 0; k < K; k++) {
			r = _rk_mm_moveldup_pf(*s);                              // [a  a  x  x ]
			i = _rk_mm_movehdup_pf(*s);                              // [b  b  y  y ]
			x = _rk_mm_shuffle_pf(*d, *d, _MM_SHUFFLE(2, 3, 0, 1));  // [d  c  w  z ]
			i = _rk_mm_mul_pf(i, x);                                 // [bd bc yw yz]
			*d = _rk_mm_fmaddsub_pf(r, *d, i);                       // [a a x x] * [c d z w] -/+ [bd bc yw yz] = [ac-bd ad+bc xz-yw xw+yz]
			s++;
			d++;
		}
#endif
	} else {
		for (k = 0; k < K; k++) {
			r = _rk_mm_moveldup_pf(*s);                              // [a  a  x  x ]
			i = _rk_mm_movehdup_pf(*s);                              // [b  b  y  y ]
			x = _rk_mm_shuffle_pf(*d, *d, _MM_SHUFFLE(2, 3, 0, 1));  // [d  c  w  z ]
			i = _rk_mm_mul_pf(i, x);                                 // [bd bc yw yz]
			*d = _rk_mm_fmaddsub_pf(r, *d, i);                       // [a a x x] * [c d z w] -/+ [bd bc yw yz] = [ac-bd ad+bc xz-yw xw+yz]
			s++;
			d++;
		}
	}
	return;
}

void RKSIMD_iyscl(RKComplex *src, const RKFloat m, const int n) {
    int k, K = (n * sizeof(RKComplex) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *s = (RKVec *)src;
    RKVec mv = _rk_mm_set1_pf(m);
    for (k = 0; k < K; k++) {
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

void RKSIMD_Int2Complex(RKInt16C *src, RKComplex *dst, const int n) {
    int k;
#if defined(__AVX512F__) || defined(__AVX2__)
    RKVecCvt *s = (RKVecCvt *)src;
    RKVec *d = (RKVec *)dst;
    int K = (n * sizeof(RKComplex) + sizeof(RKVec) - 1) / sizeof(RKVec);
    for (k = 0; k < K; k++) {
        *d++ = _rk_mm_cvtepi32_pf(_rk_mm_cvtepi16_epi32(*s++));
    }
#else
    __m128i *s = (__m128i *)src;
    __m128  *d = (__m128 *) dst;
    __m128i  z = _mm_setzero_si128();
    int K = (n * sizeof(RKFloat) + sizeof(__m128) - 1) / sizeof(__m128);
    for (k = 0; k < K; k++) {
        *d++ = _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_unpacklo_epi16(*s, z)));
        *d++ = _mm_cvtepi32_ps(_mm_cvtepi16_epi32(_mm_unpackhi_epi16(*s++, z)));
    }
#endif
    return;
}

void RKSIMD_Int2Complex_reg(RKInt16C *src, RKComplex *dst, const int n) {
    for (int i = 0; i < n; i++) {
        dst[i].i = (RKFloat)src[i].i;
        dst[i].q = (RKFloat)src[i].q;
    }
    return;
}

// Subtract by a float
void RKSIMD_subc(RKFloat *src, const RKFloat f, RKFloat *dst, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    const RKVec fv = _rk_mm_set1_pf(f);
    RKVec *s = (RKVec *)src;
    RKVec *d = (RKVec *)dst;
    for (k = 0; k < K; k++) {
        *d++ = _rk_mm_sub_pf(*s++, fv);
    }
    return;
}

// Specialized functions: normalize C(0) by u, then get sqrt(|C(0)| / |Rh(0) * Rv(0)|)
void RKSIMD_izrmrm(RKIQZ *src, RKFloat *dst, RKFloat *x, RKFloat *y, RKFloat u, const int n) {
    int k, K = (n * sizeof(RKFloat) + sizeof(RKVec) - 1) / sizeof(RKVec);
    RKVec *si = (RKVec *)src->i;
    RKVec *sq = (RKVec *)src->q;
    RKVec *a = (RKVec *)x;
    RKVec *b = (RKVec *)y;
    RKVec *d = (RKVec *)dst;
    RKVec u_pf = _rk_mm_set1_pf(u);
    RKVec m;
    for (k = 0; k < K; k++) {
        *si = _rk_mm_mul_pf(*si, u_pf);
        *sq = _rk_mm_mul_pf(*sq, u_pf);
        m = _rk_mm_rcp_pf(_rk_mm_mul_pf(*a++, *b++));  // 1.0 / (|Rh(0)| * |Rv(0)|)
        *d++ = _rk_mm_sqrt_pf(_rk_mm_mul_pf(_rk_mm_add_pf(_rk_mm_mul_pf(*si, *si), _rk_mm_mul_pf(*sq, *sq)), m));
        si++;
        sq++;
    }
    return;
}
