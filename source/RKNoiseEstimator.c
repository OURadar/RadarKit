int RKRayNoiseEstimator(RKMomentScratch *space, RKPulse **pulses, const uint16_t pulseCount) {
    int n, j, k, p;
    RKFloat mS;
    RKPulse *pulse = pulses[0];
    const uint32_t gateCount = pulse->header.downSampledGateCount;
    for (p = 0; p < 2; p++) {

        // Initializes the storage
        RKZeroOutIQZ(&space->mX[p], space->capacity);
        RKZeroOutIQZ(&space->R[p][0], space->capacity);

        RKIQZ *R = &space->R[p][0];

        // Go through all pulses
        n = 0;
        do {
            RKIQZ Xn = RKGetSplitComplexDataFromPulse(pulses[n], p);

            // Go through each lag
            RKIQZ Xk = RKGetSplitComplexDataFromPulse(pulses[n], p);
            RKSIMD_zcma(&Xn, &Xk, &R[0], gateCount, 1);                          // R[k] += X[n] * X[n - k]'
            n++;
        } while (n != pulseCount);


        RKSIMD_izscl(&R[0], 1.0 / ((float)(n)), gateCount);                      // R[k] /= (n - k)   (unbiased)
        RKSIMD_zabs(&R[0], space->aR[p][0], gateCount);                              // aR[k] = abs(R[k])

        // // Mean and variance (2nd moment)
        // RKSIMD_zsmul(&space->mX[p], &space->vX[p], gateCount, 1);                        // E{Xh} * E{Xh}' --> var  (step 1)
        // RKSIMD_izsub(&space->R[p][0], &space->vX[p], gateCount);                         // Rh[] - var     --> var  (step 2)
    }


    for (p = 0; p < 2; p++) {
        mS = 0
        for (k = 0; k < gateCount; k++) {
            mS += space->aR[p][0][k]/gateCount;
        }
        for (k = 0; k < gateCount; k++) {
            space->S[p][k] = space->aR[p][0][k]/mS;                            // S[p][k] is iq_power
        }
    }
    for (k = 0; k < gateCount; k++) {
        space->mask[k] = 1;
    }
}