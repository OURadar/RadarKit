//
//  RKTest.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/25/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKTest.h>
#include <getopt.h>

#define RKFMT  "%4d"

void RKTestModuloMath(void) {
    int k;
    const int N = 4;

    RKLog("Test with SlotCount = %d, N = %d\n", RKBuffer0SlotCount, N);
    k = 0;                      RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 4; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 3; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 2; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 1; RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, N, RKBuffer0SlotCount));
    k = RKBuffer0SlotCount - 1; RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 0;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 1;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 2;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 3;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    k = 4;                      RKLog("k = " RKFMT " --> Prev N = " RKFMT "\n", k, RKPreviousNModuloS(k, N, RKBuffer0SlotCount));
    
    k = 4899;                   RKLog("k = " RKFMT " --> Next N = " RKFMT "\n", k, RKNextNModuloS(k, 100 - 1, RKBuffer0SlotCount));
}


#define RKSIMD_TEST_DESC_FORMAT        "%65s"
#define RKSIMD_TEST_RESULT(str, res)   printf(RKSIMD_TEST_DESC_FORMAT " : %s.\033[0m\n", str, res ? "\033[32msuccessful" : "\033[31mfailed");
#define OXSTR(x)                       x ? "\033[32mo\033[0m" : "\033[31mx\033[0m"

void RKTestSIMD(const RKTestSIMDFlag flag) {
    RKSIMD_show_info();
    
    int i;
    const int n = 32;
    
    // PKIQZ struct variables are usually allocated somewhere else
    RKIQZ s, d, c;

    // In a local context, they are usually in reference form
    RKIQZ *src = &s;
    RKIQZ *dst = &d;
    RKIQZ *cpy = &c;
    RKComplex *cs;
    RKComplex *cd;
    RKComplex *cc;
    
    posix_memalign((void **)&src->i, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat));
    posix_memalign((void **)&src->q, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat));
    posix_memalign((void **)&dst->i, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat));
    posix_memalign((void **)&dst->q, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat));
    posix_memalign((void **)&cpy->i, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat));
    posix_memalign((void **)&cpy->q, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat));
    posix_memalign((void **)&cs, RKSIMDAlignSize, RKGateCount * sizeof(RKComplex));
    posix_memalign((void **)&cd, RKSIMDAlignSize, RKGateCount * sizeof(RKComplex));
    posix_memalign((void **)&cc, RKSIMDAlignSize, RKGateCount * sizeof(RKComplex));

    const RKFloat tiny = 1.0e-3f;
    bool good;
    bool all_good = true;
    
    //
    
    for (i = 0; i < n; i++) {
        src->i[i] = (RKFloat)i;
        src->q[i] = (RKFloat)-i;
    }
    
    RKSIMD_zcpy(src, dst, n);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        good = src->i[i] == dst->i[i] && src->q[i] == dst->q[i];
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("src[%2d] = %9.2f%+9.2fi   dst[%2d] = %9.2f%+9.2fi\n", i, src->i[i], src->q[i], i, dst->i[i], dst->q[i]);
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Copy -  zcpy", all_good);
    
    //
    
    memset(dst->i, 0, n * sizeof(RKFloat));
    memset(dst->q, 0, n * sizeof(RKFloat));
    
    RKSIMD_zscl(src, 3.0f, dst, n);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 3-3i, 6-6i, 9-9i, ...
        good = fabsf(dst->i[i] - 3.0f * i) < tiny && fabs(dst->q[i] + 3.0f * i) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi x 3.0 -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Scaling by a Float -  zscl", all_good);
    
    //
    
    RKSIMD_zadd(src, src, dst, n);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Addition -  zadd", all_good);
    
    //
    
    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izadd(src, dst, n);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (int i = 0; i < n; i++) {
        // Answers should be 0, 2-2i, 3-3i, 4-4i, ...
        good = fabsf(dst->i[i] - (float)(2 * i)) < tiny && fabs(dst->q[i] - (float)(-2 * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ++ -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("In-place Complex Vector Addition - izadd", all_good);
    
    //
    
    RKSIMD_zmul(src, src, dst, n, false);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Complex Vector Multiplication -  zmul", all_good);
    
    //
    
    RKSIMD_zcpy(src, dst, n);
    RKSIMD_izmul(src, dst, n, false);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, -2i, -8i, -18i, ...
        good = fabsf(dst->i[i]) < tiny && fabs(dst->q[i] - (float)(-2 * i * i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%9.2f%+9.2fi ** -> %9.2f%+9.2fi  %s\n", src->i[i], src->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("In-place Complex Vector Multiplication - izmul", all_good);
    
    //
    
    // Populate some numbers
    for (i = 0; i < n; i++) {
        cs[i].i = (RKFloat)i;
        cs[i].q = (RKFloat)(-i);
        cd[i].i = (RKFloat)(i + 1);
        cd[i].q = (RKFloat)(-i);
    }
    memcpy(cc, cd, n * sizeof(RKComplex));
    
    RKSIMD_iymul(cs, cd, n);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 1-3i, 2-10i, 3-21i, ...
        good = fabsf(cd[i].i - (RKFloat)i) < tiny && fabsf(cd[i].q - (RKFloat)(-2 * i * i - i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
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
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0, 1-3i, 2-10i, 3-21i, ...
        good = fabsf(cc[i].i - (RKFloat)i) < tiny && fabsf(cc[i].q - (RKFloat)(-2 * i * i - i)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%+9.2f%+9.2f * %+9.2f%+9.2f = %+9.2f%+9.2f  %s\n", src->i[i], src->q[i], cpy->i[i], cpy->q[i], dst->i[i], dst->q[i], OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Deinterleave, Multiply Using iymul, and Interleave", all_good);
    
    //
    
    RKInt16C *is = (RKInt16C *)src->i;
    
    for (i = 0; i < n; i++) {
        is[i].i = i;
        is[i].q = i - 1;
    }
    memset(cd, 0, n * sizeof(RKComplex));
    
    RKSIMD_Int2Complex_reg(is, cd, n);
    
    if (flag & RKTestSIMDFlagShowNumbers) {
        printf("====\n");
    }
    all_good = true;
    for (i = 0; i < n; i++) {
        // Answers should be 0-1i, 1-2i, 2-3i, 3-41i, ...
        good = fabsf(cd[i].i - (RKFloat)i) < tiny && fabsf(cd[i].q - (RKFloat)(i - 1)) < tiny;
        if (flag & RKTestSIMDFlagShowNumbers) {
            printf("%+3d%+3di -> %+5.1f%+5.1f  %s\n", is[i].i, is[i].q, cd[i].i, cd[i].q, OXSTR(good));
        }
        all_good &= good;
    }
    RKSIMD_TEST_RESULT("Conversion from i16 to float", all_good);
    
    if (flag & RKTestSIMDFlagPerformanceTestAll) {
        printf("\n==== Performance Test ====\n\n");
        printf("Using %d gates\n", RKGateCount);
        
        int k;
        const int m = 100000;
        struct timeval t1, t2;
        
        if (flag & RKTestSIMDFlagPerformanceTestArithmetic) {
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
            printf("              -Os: %.3fs (Compiler Optimized)\n", RKTimevalDiff(t2, t1));
            
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
        }
        
        if (flag & RKTestSIMDFlagPerformanceTestConversion) {
            printf("Copy (%dK loops):\n", m / 1000);
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                memcpy(src->i, dst->i, RKGateCount * sizeof(RKFloat));
                memcpy(src->q, dst->q, RKGateCount * sizeof(RKFloat));
            }
            gettimeofday(&t2, NULL);
            printf("       memcpy x 2: %.3fs (Compiler Optimized)\n", RKTimevalDiff(t2, t1));
            
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_zcpy(src, dst, RKGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("             zcpy: %.3fs (SIMD)\n", RKTimevalDiff(t2, t1));
            
            printf("Conversions (%dK loops):\n", m / 1000);
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_Int2Complex_reg(is, cd, RKGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("              -Os: %.3fs (Compiler Optimized)\n", RKTimevalDiff(t2, t1));
            
            gettimeofday(&t1, NULL);
            for (k = 0; k < m; k++) {
                RKSIMD_Int2Complex(is, cd, RKGateCount);
            }
            gettimeofday(&t2, NULL);
            printf("      cvtepi32_ps: %.3fs (SIMD)\n", RKTimevalDiff(t2, t1));
        }
        
        printf("\n==========================\n");
    }
    
    free(src->i);
    free(src->q);
    free(dst->i);
    free(dst->q);
    free(cpy->i);
    free(cpy->q);
    free(cs);
    free(cd);
    free(cc);
}

RKTransceiver RKTestSimulateDataStream(RKRadar *radar, void *input) {
    int j, g, p;
    float phi = 0.0f;
    float tau = 0.0f;
    float azimuth = 0.0f;
    struct timeval t0, t1;
    double dt = 0.0;
    double prt = 0.0002;
    float fs = 50.0e6;
    int n = 0;

    gettimeofday(&t0, NULL);

    RKSetLogfile(NULL);

    // Parse out input parameters
    if (input) {
        char *sb = (char *)input, *se = NULL, *sv = NULL;
        while (*sb == ' ') {
            sb++;
        }
        while ((se = strchr(sb, ' ')) != NULL) {
            sv = se + 1;
            switch (*sb) {
                case 'f':
                    prt = 1.0 / (double)atof(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">prf = %s Hz", RKIntegerToCommaStyleString((long)(1.0f / prt)));
                    }
                    break;
                case 'F':
                    fs = atof(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">fs = %s Hz", RKIntegerToCommaStyleString((long)fs));
                    }
                    break;
            }
            sb = strchr(sv, ' ');
            if (sb == NULL) {
                break;
            } else {
                while (*sb == ' ') {
                    sb++;
                }
            }
        }
    }

    //const int gateCount = RKGetPulseCapacity(radar);
    const int gateCount = 6;
    const int chunkSize = MAX(1, (int)floor(0.1f / prt));

    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("<RKTestSimulateDataStream> fs = %s MHz   PRF = %s Hz   gateCount = %s (%.1f km)   chunk %d\n",
              RKFloatToCommaStyleString(1.0e-6 * fs),
              RKIntegerToCommaStyleString((int)(1.0f / prt)),
              RKIntegerToCommaStyleString(gateCount),
              gateCount * 1.5e5 / fs,
              chunkSize);
    }

    while (radar->active) {

        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("%s %s",
                  RKPulseCompressionEngineStatusString(radar->pulseCompressionEngine),
                  RKMomentEngineStatusString(radar->momentEngine));
        }

        for (j = 0; radar->active && j < chunkSize; j++) {
            RKPulse *pulse = RKGetVacantPulse(radar);
            // Fill in the header
            pulse->header.gateCount = gateCount;
            pulse->header.azimuthDegrees = azimuth;
            pulse->header.elevationDegrees = 2.41f;

            // Stamp the pulse

            // Fill in the data...
            for (p = 0; p < 2; p++) {
                RKInt16C *X = RKGetInt16CDataFromPulse(pulse, p);
                // Some seemingly random pattern for testing
                n = pulse->header.i % 3 * (pulse->header.i % 2 ? 1 : -1) + p;
                for (g = 0; g < gateCount; g++) {
                    //X->i = (int16_t)(32767.0f * cosf(phi * (float)k));
                    //X->q = (int16_t)(32767.0f * sinf(phi * (float)k));
                    if (g % 2 == 0) {
                        X->i = (int16_t)(g * n) + p;
                        X->q = (int16_t)((n - 2) * (g - 1));
                    } else {
                        X->i = (int16_t)(-g * (n - 1));
                        X->q = (int16_t)(g * p) + n;
                    }
                    X++;
                }
            }
            phi += 0.02f;
            //azimuth = fmodf(50.0f * tau, 360.0f);
            azimuth = fmodf(1.0f * tau, 360.0f);

            RKSetPulseHasData(pulse);

            tau += prt;
        }

        // Wait to simulate the PRF
        n = 0;
        do {
            gettimeofday(&t1, NULL);
            dt = RKTimevalDiff(t1, t0);
            usleep(1000);
            n++;
        } while (radar->active && dt < prt * chunkSize);
        t0 = t1;
    }
    return NULL;
}

void RKTestPulseCompression(RKRadar *radar, RKTestFlag flag) {
    RKPulse *pulse = RKGetVacantPulse(radar);
    pulse->header.gateCount = 6;

    RKInt16C *X = RKGetInt16CDataFromPulse(pulse, 0);
    memset(X, 0, pulse->header.gateCount * sizeof(RKInt16C));
    X[0].i = 1;
    X[0].q = 0;
    X[1].i = 2;
    X[1].q = 0;
    X[2].i = 4;
    X[2].q = 0;
    X[3].i = 2;
    X[3].q = 0;
    X[4].i = 1;
    X[4].q = 0;
    RKSetPulseReady(pulse);

    while ((pulse->header.s & RKPulseStatusCompressed) == 0) {
        usleep(1000);
    }

    RKComplex *F = &radar->pulseCompressionEngine->filters[0][0][0];
    RKComplex *Y = RKGetComplexDataFromPulse(pulse, 0);
    RKIQZ Z = RKGetSplitComplexDataFromPulse(pulse, 0);

    if (flag & RKTestFlagShowResults) {
        printf("X =                       F =                     Y =                             Z =\n");
        for (int k = 0; k < 8; k++) {
            printf("    [ %6d %s %6di ]      [ %5.2f %s %5.2fi ]      [ %9.2f %s %9.2fi ]      [ %9.2f %s %9.2fi ]\n",
                   X[k].i, X[k].q < 0 ? "-" : "+", abs(X[k].q),
                   F[k].i, F[k].q < 0.0f ? "-" : "+", fabs(F[k].q),
                   Y[k].i, Y[k].q < 0.0f ? "-" : "+", fabs(Y[k].q),
                   Z.i[k], Z.q[k] < 0.0f ? "-" : "+", fabs(Z.q[k]));
        }
    }
}
