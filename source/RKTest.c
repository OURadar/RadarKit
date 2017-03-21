//
//  RKTest.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 10/25/16.
//  Copyright (c) 2016 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKTest.h>
#include <getopt.h>

#define RKFMT                          "%4d"
#define RKSIMD_TEST_DESC_FORMAT        "%65s"
#define RKSIMD_TEST_RESULT(str, res)   printf(RKSIMD_TEST_DESC_FORMAT " : %s.\033[0m\n", str, res ? "\033[32msuccessful" : "\033[31mfailed");
#define OXSTR(x)                       x ? "\033[32mo\033[0m" : "\033[31mx\033[0m"


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


void RKTestSIMD(const RKTestSIMDFlag flag) {
    RKSIMD_show_info();
    
    int i;
    const int n = 32;

    for (i = 1; i <= 8; i++) {
        RKSIMD_show_count(i);
    }

    // PKIQZ struct variables are usually allocated somewhere else
    RKIQZ s, d, c;

    // In a local context, they are usually in reference form
    RKIQZ *src = &s;
    RKIQZ *dst = &d;
    RKIQZ *cpy = &c;
    RKComplex *cs;
    RKComplex *cd;
    RKComplex *cc;
    
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&src->i, RKSIMDAlignSize, n * sizeof(RKFloat)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&src->q, RKSIMDAlignSize, n * sizeof(RKFloat)))
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&dst->i, RKSIMDAlignSize, n * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&dst->q, RKSIMDAlignSize, n * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cpy->i, RKSIMDAlignSize, n * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cpy->q, RKSIMDAlignSize, n * sizeof(RKFloat)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cs,     RKSIMDAlignSize, n * sizeof(RKComplex)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cd,     RKSIMDAlignSize, n * sizeof(RKComplex)));
    POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cc,     RKSIMDAlignSize, n * sizeof(RKComplex)));

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
        printf("Using %s gates\n", RKIntegerToCommaStyleString(RKGateCount));
        free(src->i);
        free(src->q);
        free(dst->i);
        free(dst->q);
        free(cpy->i);
        free(cpy->q);
        free(cs);
        free(cd);
        free(cc);
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&src->i, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&src->q, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&dst->i, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&dst->q, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cpy->i, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cpy->q, RKSIMDAlignSize, RKGateCount * sizeof(RKFloat)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cs,     RKSIMDAlignSize, RKGateCount * sizeof(RKComplex)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cd,     RKSIMDAlignSize, RKGateCount * sizeof(RKComplex)));
        POSIX_MEMALIGN_CHECK(posix_memalign((void **)&cc,     RKSIMDAlignSize, RKGateCount * sizeof(RKComplex)));

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

void RKTestParseCommaDelimitedValues(void) {
    char string[] = "1200,2000,3000,5000";
    float v[4];
    int32_t i[4];
    RKParseCommaDelimitedValues(v, RKValueTypeFloat, 3, string);
    RKLog("%s -> %.2f %.2f %.2f\n", string, v[0], v[1], v[2]);
    RKParseCommaDelimitedValues(i, RKValueTypeInt32, 3, string);
    RKLog("%s -> %d %d %d\n", string, i[0], i[1], i[2]);
}


#pragma mark - Simulated Transceiver

void *RKTestTransceiverRunLoop(void *input) {
    RKTestTransceiver *transceiver = (RKTestTransceiver *)input;
    RKRadar *radar = transceiver->radar;

    int j, p, g, n;
    float a;
    float phi = 0.0f;
    float azimuth = 0.0f;
    double t = 0.0f;
    double dt = 0.0;
    struct timeval t0, t1;

    
    const int chunkSize = MAX(1, (int)floor(0.25 / transceiver->prt));
    
    if (radar->desc.initFlags & RKInitFlagVerbose) {
        RKLog("%s fs = %s MHz   PRF = %s Hz   gateCount = %s   range = %.1f km\n",
              transceiver->name,
              RKFloatToCommaStyleString(1.0e-6 * transceiver->fs),
              RKIntegerToCommaStyleString((int)(1.0f / transceiver->prt)),
              RKIntegerToCommaStyleString(transceiver->gateCount),
              transceiver->gateCount * transceiver->gateSizeMeters * 1.0e-3);
        RKLog("%s chunk size = %s   tics = %s\n",
              transceiver->name,
              RKIntegerToCommaStyleString(chunkSize),
              RKFloatToCommaStyleString(1.0e6 * transceiver->prt));
    }
    
    gettimeofday(&t0, NULL);

    transceiver->state |= RKEngineStateActive;

    while (transceiver->state & RKEngineStateActive) {
        
        for (j = 0; j < chunkSize && transceiver->state & RKEngineStateActive; j++) {
            RKPulse *pulse = RKGetVacantPulse(radar);
            
            // Fill in the header
            pulse->header.i = transceiver->counter++;
            pulse->header.t = (uint64_t)(1.0e6 * t);
            pulse->header.gateCount = transceiver->gateCount;
            pulse->header.gateSizeMeters = transceiver->gateSizeMeters;
            
            if (transceiver->simulatePosition) {
                pulse->header.azimuthDegrees = azimuth;
                pulse->header.elevationDegrees = 2.41f;
                azimuth = fmodf(25.0f * t, 360.0f);
                phi += 0.02f;
            }
            
            a = cosf(2.0 * M_PI * t);
            // Fill in the data...
            for (p = 0; p < 2; p++) {
                RKInt16C *X = RKGetInt16CDataFromPulse(pulse, p);
                
                // Some seemingly random pattern for testing
                //n = pulse->header.i % 3 * (pulse->header.i % 2 ? 1 : -1) + p;
                for (g = 0; g < transceiver->gateCount; g++) {
                    X->i = (int16_t)(1000.0f * a * cosf((float)g * transceiver->gateSizeMeters * 0.0001f));
                    X->q = 0.0f;
                    X++;
                }
            }
            
            RKSetPulseHasData(radar, pulse);
            
            if (transceiver->sleepInterval > 0 && transceiver->counter % transceiver->sleepInterval == 0) {
                RKLog("%s sleeping at counter = %s / %s ...",
                      transceiver->name, RKIntegerToCommaStyleString(transceiver->counter), RKIntegerToCommaStyleString(transceiver->sleepInterval));
                sleep(3);
                transceiver->counter = 0;
            }
            t += transceiver->prt;
        }

        // Report health
        RKHealth *health = RKGetVacantHealth(radar, RKHealthNodeTransceiver);
        float temp = 1.0f * rand() / RAND_MAX + 79.5f;
        float volt = 1.0f * rand() / RAND_MAX + 11.5f;
        sprintf(health->string,
                "{\"Trigger\":{\"Value\":true,\"Enum\":0}, "
                "\"Ready\":{\"Value\":true,\"Enum\":0}, "
                "\"FPGA Temp\":{\"Value\":\"%.1fdegC\",\"Enum\":%d}, "
                "\"XMC Voltage\":{\"Value\":\"%.1f V\",\"Enum\":%d}, "
                "\"GPS Latitude\":{\"Value\":\"%.6f\",\"Enum\":0}, "
                "\"GPS Longitude\":{\"Value\":\"%.6f\",\"Enum\":0}, "
                "\"GPS Heading\":{\"Value\":\"%.6f\",\"Enum\":0}, "
                "\"NULL\": %ld}",
                temp, temp > 80.0f ? 1 : 0,
                volt, volt > 12.2f ? 1 : 0,
                (double)rand() * 1.0 / RAND_MAX + 35,
                (double)rand() * 1.0 / RAND_MAX - 95,
                (double)rand() * 1.0 / RAND_MAX + 45,
                transceiver->counter);
        RKSetHealthReady(radar, health);

        // Wait to simulate the PRF
        n = 0;
        do {
            gettimeofday(&t1, NULL);
            dt = RKTimevalDiff(t1, t0);
            usleep(100);
            n++;
        } while (radar->active && dt < transceiver->prt * chunkSize);
        t0 = t1;
    }
    return NULL;
}


RKTransceiver RKTestTransceiverInit(RKRadar *radar, void *input) {

    RKTestTransceiver *transceiver = (RKTestTransceiver *)malloc(sizeof(RKTestTransceiver));
    if (transceiver == NULL) {
        RKLog("Error. Unable to allocate a test transceiver.\n");
        exit(EXIT_FAILURE);
    }
    memset(transceiver, 0, sizeof(RKTestTransceiver));
    sprintf(transceiver->name, "%s<Transceiver>%s",
            rkGlobalParameters.showColor ? RKGetBackgroundColor() : "", rkGlobalParameters.showColor ? RKNoColor : "");
    transceiver->radar = radar;
    transceiver->fs = 5.0e6;
    transceiver->gateCount = RKGetPulseCapacity(radar);
    transceiver->state = RKEngineStateAllocated;
    
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
                    transceiver->prt = 1.0 / (double)atof(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">prf = %s Hz", RKIntegerToCommaStyleString((long)(1.0f / transceiver->prt)));
                    }
                    break;
                case 'F':
                    transceiver->fs = atof(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">fs = %s Hz", RKIntegerToCommaStyleString((long)transceiver->fs));
                    }
                    break;
                case 'g':
                    transceiver->gateCount = atoi(sv);
                    uint32_t capacity = RKGetPulseCapacity(radar);
                    if (transceiver->gateCount > capacity) {
                        RKLog("Warning. gateCount %s will be clamped to the capacity %s",
                              RKIntegerToCommaStyleString(transceiver->gateCount), RKIntegerToCommaStyleString(capacity));
                        transceiver->gateCount = capacity;
                    }
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">gateCount = %s", RKIntegerToCommaStyleString(transceiver->gateCount));
                    }
                    break;
                case 'P':
                    transceiver->simulatePosition = true;
                    break;
                case 'z':
                    transceiver->sleepInterval = atoi(sv);
                    if (radar->desc.initFlags & RKInitFlagVeryVeryVerbose) {
                        RKLog(">sleepInterval = %s", RKIntegerToCommaStyleString(transceiver->sleepInterval));
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

    // Derive some calculated parameters
    transceiver->gateSizeMeters = 60.0e3f / transceiver->gateCount;

    // Use a counter that mimics microsend increments
    RKSetPulseTicsPerSeconds(radar, 1.0e6);

    transceiver->state |= RKEngineStateActivating;
    if (pthread_create(&transceiver->tidRunLoop, NULL, RKTestTransceiverRunLoop, transceiver)) {
        RKLog("%s. Unable to create transceiver run loop.\n", transceiver->name);
    }
    while (!(transceiver->state & RKEngineStateActive)) {
        usleep(10000);
    }
    
    return (RKTransceiver)transceiver;
}

int RKTestTransceiverExec(RKTransceiver transceiverReference, const char *command, char *response) {
    RKTestTransceiver *transceiver = (RKTestTransceiver *)transceiverReference;
    RKRadar *radar = transceiver->radar;
    if (response != NULL) {
        sprintf(response, "NAK. Command not understood." RKEOL);
    }
    if (!strcmp(command, "disconnect")) {
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("%s Disconnecting ...", transceiver->name);
        }
        transceiver->state |= RKEngineStateDeactivating;
        transceiver->state ^= RKEngineStateActive;
        pthread_join(transceiver->tidRunLoop, NULL);
        if (response != NULL) {
            sprintf(response, "ACK. Transceiver stopped." RKEOL);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("%s Stopped.\n", transceiver->name);
        }
        transceiver->state = RKEngineStateAllocated;
    } else if (!strncmp(command, "prt", 3)) {
        transceiver->prt = atof(command + 3);
        if (response != NULL) {
            sprintf(response, "ACK. PRT = %.3f ms" RKEOL, 1.0e3 * transceiver->prt);
        }
        if (radar->desc.initFlags & RKInitFlagVerbose) {
            RKLog("%s PRT = %s\n", transceiver->name, RKFloatToCommaStyleString(transceiver->prt));
        }
    } else if (command[0] == 'h') {
        sprintf(response,
                "Commands:\n"
                "---------\n"
                UNDERLINE("help") " - Help list.\n"
                UNDERLINE("prt") " [value] - PRT set to value\n"
                UNDERLINE("z") " [value] - Sleep interval set to value.\n"
                );
    } else if (command[0] == 'z') {
        transceiver->sleepInterval = atoi(command + 1);
        RKLog("%s sleepInterval = %s", transceiver->name, RKIntegerToCommaStyleString(transceiver->sleepInterval));
    }
    return RKResultSuccess;
}

int RKTestTransceiverFree(RKTransceiver transceiverReference) {
    RKTestTransceiver *transceiver = (RKTestTransceiver *)transceiverReference;
    free(transceiver);
    return RKResultSuccess;
}

#pragma mark - Data Processing

void RKTestPulseCompression(RKRadar *radar, RKTestFlag flag) {
    RKPulse *pulse;
    RKInt16C *X;
    RKComplex *F;
    RKComplex *Y;
    RKIQZ Z;

    // Change filter to [1, 1i] for case 2
    RKComplex filter[] = {{1.0f, 0.0f}, {0.0f, 1.0f}};

    for (int k = 0; k < 3; k++) {
        printf("\n");

        switch (k) {
            case 1:
                // Range average [1 1]
                printf("Filter [1, 1]:\n\n");
                RKPulseCompressionSetFilterTo11(radar->pulseCompressionEngine);
                break;
            case 2:
                // Change filter to [1, 1i]
                printf("Filter [1, i]:\n\n");
                RKFilterAnchor anchor = RKFilterAnchorDefaultWithMaxDataLength(8);
                RKPulseCompressionSetFilter(radar->pulseCompressionEngine, filter, anchor, 0, 0);
                break;
            default:
                // Default is impulse [1];
                printf("Pulse filter [1]:\n\n");
                RKPulseCompressionSetFilterToImpulse(radar->pulseCompressionEngine);
                break;
        }

        pulse = RKGetVacantPulse(radar);
        pulse->header.gateCount = 6;

        X = RKGetInt16CDataFromPulse(pulse, 0);
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
        RKSetPulseReady(radar, pulse);

        while ((pulse->header.s & RKPulseStatusCompressed) == 0) {
            usleep(1000);
        }

        F = &radar->pulseCompressionEngine->filters[0][0][0];
        Y = RKGetComplexDataFromPulse(pulse, 0);
        Z = RKGetSplitComplexDataFromPulse(pulse, 0);

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
}

// Make some private functions available

int makeRayFromScratch(RKScratch *, RKRay *, const int gateCount, const int stride);

void RKTestProcessorSpeed(void) {
    int k;
    RKScratch *space;
    RKBuffer pulseBuffer;
    RKBuffer rayBuffer;
    const int testCount = 1000;
    const int pulseCount = 100;
    const int pulseCapacity = 4096;

    RKPulseBufferAlloc(&pulseBuffer, pulseCapacity, pulseCount);
    RKRayBufferAlloc(&rayBuffer, pulseCapacity, 1);

    RKScratchAlloc(&space, pulseCapacity, 4, true);

    RKPulse *pulses[pulseCount];
    for (k = 0; k < pulseCount; k++) {
        RKPulse *pulse = RKGetPulse(pulseBuffer, k);
        pulse->header.t = k;
        pulse->header.gateCount = pulseCapacity;
        pulses[k] = pulse;
    }

    struct timeval tic, toc;

    RKRay *ray = RKGetRay(rayBuffer, 0);
    
    gettimeofday(&tic, NULL);
    for (k = 0; k < testCount; k++) {
        RKPulsePairHop(space, pulses, pulseCount);
        //RKMultiLag(space, pulses, pulseCount);
         makeRayFromScratch(space, ray, pulseCapacity, 1);
    }
    gettimeofday(&toc, NULL);

    double t = RKTimevalDiff(toc, tic);
    RKLog("Total elapsed time: %.3f s\n", t);
    RKLog("Time for each ray (%s pulses x %s gates) = %.3f ms\n",
          RKIntegerToCommaStyleString(pulseCount),
          RKIntegerToCommaStyleString(pulseCapacity),
          1.0e3 * t / testCount);

    RKScratchFree(space);
    free(pulseBuffer);
    free(rayBuffer);
    return;
}

void RKTestOneRay(void) {
    int k, p, n, g;
    RKScratch *space;
    RKBuffer pulseBuffer;
    const int gateCount = 6;
    const int pulseCount = 10;
    const int pulseCapacity = 64;
    
    RKLog("Allocating buffers ...\n");
    
    RKPulseBufferAlloc(&pulseBuffer, pulseCapacity, pulseCount);
    RKScratchAlloc(&space, pulseCapacity, 4, true);
    RKPulse *pulses[pulseCount];
    
    for (k = 0; k < pulseCount; k++) {
        RKPulse *pulse = RKGetPulse(pulseBuffer, k);
        pulse->header.i = k;
        pulse->header.t = k;
        pulse->header.gateCount = gateCount;
        // Fill in the data...
        for (p = 0; p < 2; p++) {
            RKIQZ X = RKGetSplitComplexDataFromPulse(pulse, p);
            
            // Some seemingly random pattern for testing
            n = pulse->header.i % 3 * (pulse->header.i % 2 ? 1 : -1) + p;
            for (g = 0; g < gateCount; g++) {
                if (g % 2 == 0) {
                    X.i[g] = (int16_t)((g * n) + p);
                    X.q[g] = (int16_t)((n - 2) * (g - 1));
                } else {
                    X.i[g] = (int16_t)(-g * (n - 1));
                    X.q[g] = (int16_t)((g * p) + n);
                }
            }
        }
        pulses[k] = pulse;
    }
    
    RKPulsePairHop(space, pulses, pulseCount);
    
    RKLog("Deallocating buffers ...\n");

    RKScratchFree(space);
    free(pulseBuffer);
    return;
}

void RKTestCacheWrite(void) {
    RKFileEngine *fileEngine = RKFileEngineInit();
    fileEngine->fd = open("._testwrite", O_CREAT | O_WRONLY, 0000644);
    if (fileEngine->fd < 0) {
        RKLog("Error. Unable to open file.\n");
        exit(EXIT_FAILURE);
    }

//    RKFileEngineSetCacheSize(fileEngine, 4);
//    RKFileEngineCacheWrite(fileEngine, bytes, 4);
//    RKFileEngineCacheWrite(fileEngine, &bytes[4], 2);
//    RKFileEngineCacheWrite(fileEngine, &bytes[6], 1);
//    RKFileEngineCacheFlush(fileEngine);
  
    RKBuffer pulseBuffer;
    RKPulseBufferAlloc(&pulseBuffer, 8192, 100);
    
    struct timeval time;
    double t0, t1;
    
    gettimeofday(&time, NULL);
    t1 = (double)time.tv_sec + 1.0e-6 * (double)time.tv_usec;
    
    uint32_t len = 0;
    for (int k = 1, j = 1; k < 50000; k++) {
        RKPulse *pulse = RKGetPulse(pulseBuffer, k % 100);
        pulse->header.gateCount = 16000;
        
        len += RKFileEngineCacheWrite(fileEngine, &pulse->header, sizeof(RKPulseHeader));
        len += RKFileEngineCacheWrite(fileEngine, RKGetInt16CDataFromPulse(pulse, 0), pulse->header.gateCount * sizeof(RKInt16C));
        len += RKFileEngineCacheWrite(fileEngine, RKGetInt16CDataFromPulse(pulse, 1), pulse->header.gateCount * sizeof(RKInt16C));
        
        if (k % 2000 == 0) {
            RKFileEngineCacheFlush(fileEngine);

            gettimeofday(&time, NULL);
            t0 = (double)time.tv_sec + 1.0e-6 * (double)time.tv_usec;
            printf("Speed = %.2f MBps (%d)\n", 1.0e-6 * len / (t0 - t1), len);
            
            if (j++ % 5 == 0) {
                printf("\n");
                t1 = t0;
                len = 0;
            }
        }
    }

    close(fileEngine->fd);
  
    // Remove the files that was just created.
    system("rm -f ._testwrite");
    
    RKFileEngineFree(fileEngine);
}

void RKTestWindow(void) {
    int k;
    int n = 6;
    double param;
    RKFloat *window = (RKFloat *)malloc(n * sizeof(RKFloat));

    printf("Hamming:\n");
    RKWindowMake(window, RKWindowTypeHamming, n);
    for (k = 0; k < n; k++) {
        printf("w[%d] = %.4f\n", k, window[k]);
    }
    printf("\n");
    
    param = 0.5;
    printf("Kaiser @ %.4f:\n", param);
    RKWindowMake(window, RKWindowTypeKaiser, n, param);
    for (k = 0; k < n; k++) {
        printf("w[%d] = %.4f\n", k, window[k]);
    }
    printf("\n");
    
    param = 0.8;
    printf("Trapezoid @ %.4f:\n", param);
    RKWindowMake(window, RKWindowTypeTrapezoid, n, param);
    for (k = 0; k < n; k++) {
        printf("w[%d] = %.4f\n", k, window[k]);
    }
    printf("\n");

    free(window);
}
