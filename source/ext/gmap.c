#include <math.h>
#include <stddef.h>

// Signal mask creation flags
#define SSGSM_FILL_NEARBY  1
#define SSGSM_FILL_GAPS  2

#define OVERSAMP 2
#define SPFILTMAXPTS 30
#define LOBEDEPTHLESS 3
#define LOBEWIDTHMORE 1
#define GMAPMAXITERS 12
#define SIGMASQMAX 5
#define SIGMASQMAXRATIO 12.18
#define VELPROGRESS 0.005
#define POWPROGRESS 1.04

// #define PI 3.141592653589793
#define PI 3.14159265358979323846264338327950288419716939937510
// #define round(x) ((x)>=0?(int)((x)+0.5):(int)((x)-0.5))
#define min(x,y) (x<y)? x : y
#define max(x,y) (x>y)? x : y
#define clip(x, minVal, maxVal) (x<minVal)? minVal : ((x>maxVal)? maxVal : x)

#define MAXSIZE 100 // This seems low...
#define MAX_SPEC_SIZE 65536

// Windows
#define WIN_RECT 0
#define WIN_HAMMING 1
#define WIN_VONHANN 2
#define WIN_BLACKMAN 3
#define WIN_BLKMANEX 4
#define WIN_BLKNTTL 5

struct tWGCM {
    int iSpecSize;
    int iWinType;
    float fClutterWidth;
    float* fModelPwr;
    int iMainLobePts;
};

struct tDftConf {
    int iSpecSize;
    int iWinType;
    float *window;
    float *fCorDots;
    float *cSpec_r;
    float *cSpec_i;
    float *fWinResp;
    float *fGaussian;
    float *fNetSpec;
};

void quicksort(int a, int b, float *A) {
    int rtidx=0, ltidx=0, k=a, l=0;
    float leftarr[MAXSIZE], rtarr[MAXSIZE], pivot;
    pivot=A[a];

    if(a==b)return;

    while(k<b) {
        ++k;
        if(A[k]>A[a]) {
            leftarr[ltidx]=A[k];
            ltidx++;
        }
        else {
            rtarr[rtidx]=A[k];
            rtidx++;
        }
    }

    k=a;
    for(l=0;l<ltidx;++l)A[k++]=leftarr[l];
    A[k++]=pivot;
    for(l=0;l<rtidx;++l)A[k++]=rtarr[l];
    if(ltidx>0)quicksort(a, a+ltidx-1, A);
    if(rtidx>0)quicksort(b-rtidx+1, b, A);
}

float angle(float y, float x) {
    if (x>0) return(atanf(y/x));
    else if (y>=0 && x<0) {
        return(PI+atanf(y/x));
    }
    else if (y<0 && x<0) {
        return(-PI+atanf(y/x));
    }
    else if (y>0 && x==0) return(PI/2);
    else if (y<0 && x==0) return(-PI/2);
    else return(0);
}

void dspGMAPinitCM(float fClutterWidth_a, tDftConf *dftConf_a, tWGCM *WGCM_a) {

    int iSpecSize, iSpecHalf;
    int N, iOutPts, iOff;
    int n, m, ii, ij;
    float *cSpec_r, *cSpec_i, w, wr, wi;
    const float *cSamp;
    float *fGaussian, fPowerD, *fNetSpec, fScale, fMaxAtten;
    float *fWinResp, fPower, fWidthScale, fQuadCoef, fWinDepthDB;

    WGCM_a->iSpecSize = dftConf_a->iSpecSize;
    WGCM_a->iWinType = dftConf_a->iWinType;
    WGCM_a->fClutterWidth = fClutterWidth_a;

    iSpecSize = min(max(dftConf_a->iSpecSize, 1), MAX_SPEC_SIZE);
    // iSpecHalf = (int) floor((iSpecSize / 2));
    iSpecHalf = iSpecSize / 2;

    // cSamp = mxGetPr(mxGetCell(dftConf_a->window, *WGCM_a->iWinType));
    cSamp = dftConf_a->window;

    // cSpec_r = (float *) malloc(iSpecSize * OVERSAMP * sizeof(float));
    // cSpec_i = (float *) malloc(iSpecSize * OVERSAMP * sizeof(float));
    // fWinResp = (float *) malloc(iSpecSize * sizeof(float));
    // fGaussian = (float *) malloc(iSpecSize * sizeof(float));
    // fNetSpec = (float *) malloc(2 * iSpecSize * sizeof(float));
    cSpec_r = dftConf_a->cSpec_r;
    cSpec_i = dftConf_a->cSpec_i;
    fWinResp = dftConf_a->fWinResp;
    fGaussian = dftConf_a->fGaussian;
    fNetSpec = dftConf_a->fNetSpec;

    if (!cSpec_r || !cSpec_i || !fWinResp || !fGaussian || !fNetSpec) {
        // TODO: Do we want to check all of them?
        printf("Error! Failed to allocate.\n");
        exit(1);
    }

    N = OVERSAMP * iSpecSize;
    for (n=0; n<N; n++) {
	    cSpec_r[n]=cSpec_i[n]=0;
	    for (m=0; m<iSpecSize; m++) {
            w=-2*PI*m*n/N;
            wr=cosf(w); wi=sinf(w);
            cSpec_r[n]=cSpec_r[n]+wr*((float) cSamp[m]); /*-wi*xi[m];*/
            cSpec_i[n]=cSpec_i[n]+wi*((float) cSamp[m]);/*+wr*xi[m*/
	    }
	}


    for(ii = 0; ii<iSpecSize; ii++) {
        fPower = 0;
        for(ij = 0; ij<OVERSAMP; ij++) {
            fPower = fPower + cSpec_r[ii * OVERSAMP + ij]*cSpec_r[ii * OVERSAMP + ij]+
                    cSpec_i[ii * OVERSAMP + ij]*cSpec_i[ii * OVERSAMP + ij];
        }
        fWinResp[(ii + iSpecHalf)%iSpecSize] = fPower;
    }

    fWidthScale = fClutterWidth_a * iSpecSize;
    fWidthScale = (fWidthScale> 1.0e-4f) ? fWidthScale : 1.0e-4f;
    fQuadCoef = -2.0f / (fWidthScale*fWidthScale); // TODO: Shouldn't this be -1/(2())

    // create gaussian
    memset(fGaussian, 0, iSpecSize * sizeof(float));
    for(ii = 0; ii<iSpecSize/2; ii++) {
        fPowerD = expf(fQuadCoef*ii*ii);
        fGaussian[(iSpecHalf + ii)%iSpecSize] = fPowerD;
        fGaussian[(iSpecHalf - ii)%iSpecSize] = fPowerD;
    }

    // convolve window with gaussian to create model. Convolving because we are windowing in time domain.
    for(ii = 0; ii < (2*iSpecSize-1); ii++) {
        fNetSpec[ii] = 0;                       // set to zero before sum
        n = (ii<iSpecSize) ? (ii+1):iSpecSize;
        for(ij = (ii<iSpecSize) ? 0:(ii-iSpecSize+1); ij < n; ij++) {
            fNetSpec[ii] += fGaussian[ij] * fWinResp[ii - ij];    // convolve: multiply and accumulate
        }
    }

    iOutPts = SPFILTMAXPTS<iSpecHalf? SPFILTMAXPTS:iSpecHalf;
    iOff = 2 * iSpecHalf;
    fScale = 1 / fNetSpec[iOff];
    for(ii = 0; ii<iOutPts; ii++)
        WGCM_a->fModelPwr[ii] = fScale * fNetSpec[ii + iOff];
    for(ii = iOutPts; ii<iSpecSize; ii++)
        WGCM_a->fModelPwr[ii] = 0.0f;

    quicksort(0, iSpecSize-1, WGCM_a->fModelPwr);

    switch(WGCM_a->iWinType) {
        case WIN_HAMMING:
            fWinDepthDB = -42;
            break;
        case WIN_VONHANN:
        case WIN_BLACKMAN:
        case WIN_BLKMANEX:
            fWinDepthDB = -57;
            break;
        case WIN_RECT:
        default:
            fWinDepthDB = -13; // TODO: I think this should be the default?
            break;
    }

    // printf("SPFILTMAXPTS - LOBEWIDTHMORE = %d\n", (SPFILTMAXPTS - LOBEWIDTHMORE));
    fMaxAtten = powf(10.0f, (0.1f * (fWinDepthDB + LOBEDEPTHLESS)));
    for(ii = 0;ii<(SPFILTMAXPTS - LOBEWIDTHMORE); ii++) { // Don't use (SPFILTMAXPTS - LOBEWIDTHMORE - 2) to mimic behavior in C
        if(WGCM_a->fModelPwr[ii] < fMaxAtten) break;
    }
    WGCM_a->iMainLobePts = ii + LOBEWIDTHMORE;

    // free(cSpec_r);
    // free(cSpec_i);
    // free(fWinResp);
    // free(fGaussian);
    // free(fNetSpec);
}

// Note - data is windowed, ffted, and powered before coming in.
//        window is also normalized RMS
void dspGMAP(float *fSpecLin_a, float fClutterWidth_a, float fNoisePower_a,
        tDftConf *dftConf_a, tWGCM *WGCM_a, int interp,
        float *pfPowRemoved, float *piGapPoints) {
    // Inputs:
    // fSpecLin_a - Doppler spectrum pointer of M pulses. The data.
    //              Note this is also the output data!
    // fClutterWidth_a - assumed clutter width. Based on 0.4/va, whatever va is. Angular velocity?
    // fNoisePower_a - noise estimate, a single value.
    // dftConf_a - struct.
    //   float *iSpecSize; - equivalent to number of pulses.
    //   float *iWinType; - enum to represent window type.
    //   const float *window; - pointer to all windows, one for each type. They are normalized for power (winCoeff = sqrt(M / sum(winCoeff.*winCoeff)) * winCoeff;)
    //   float *fCorDots; - 4 × M matrix of cosine values at specific phase shifts.
    // WGCM_a - struct. These are all zeros. It checks if something has changed below and intializes it.
    //   float *iSpecSize; -
    //   float *iWinType; -
    //   float *fClutterWidth; -
    //   float *fModelPwr; - This one is an array!
    //   float *iMainLobePts; -
    // float interp - interpolate or not. set to 1.
    // float *pfPowRemoved - scalar value output, init 5
    // float *piGapPoints  - scalar value output, init 3


    // I am concerned, the output of this is not IQ data that
    // could be sent to the product generator. Does it need IQ or just the spectrum power?
    // May need to do some phase interpolation to perform the IFFT.

    int iSpecSize, lNoiseGiven, iMaxGapKill, iGapOneSide, iIndex, ii;
    float fNoiseTerm, fMin, fHold, fNoiseCutoff, fModelOff;
    float fR0Data, fR0DGap ,cR1Data_r, cR1Data_i, cR1DGap_r, cR1DGap_i;
    float fGFrac, fSpill, fTerm, *fReplace;

    int iGapPoints, iSignalPoints, iTerm, iPass, iIntVel;
    float fR0MGap, cR1MGap_r, cR1MGap_i, fSigmaSqMin, fR0, cR1_r, cR1_i;
    float fR1Mag, fR1Arg, fMeanV, fSigmaSq, fR0Scale, fVNorm;
    float fVOff, fModel, fR1ArgLast, fR0Last, fSpecSum, fPolySum, pTerm, fNewVal, fPowRemoved;

    iSpecSize = clip(dftConf_a->iSpecSize, 1, MAX_SPEC_SIZE);
    if(fNoisePower_a > 0) lNoiseGiven = 1;
    else lNoiseGiven = 0;

    if(lNoiseGiven) {
        fNoiseTerm = (float) fNoisePower_a / iSpecSize; // per bin noise
    } else {
        // mexErrMsgTxt("Noise value must be provided.");
        printf("Error! Noise value must be provided.\n");
        return;
    }

    if ((WGCM_a->fClutterWidth != fClutterWidth_a) ||
        (WGCM_a->iWinType != dftConf_a->iWinType) ||
        (WGCM_a->iSpecSize != dftConf_a->iSpecSize)) {
        dspGMAPinitCM(fClutterWidth_a, dftConf_a, WGCM_a);
    }

    fModelOff = fSpecLin_a[iSpecSize-1] + fSpecLin_a[0] + fSpecLin_a[1]; // sum DC +/-1 to estimate clutter gaussian
    fModelOff = fModelOff / (WGCM_a->fModelPwr[0] + 2*WGCM_a->fModelPwr[1]); // create ratio to gaussian model

    // find the second minimum to compare to noise power
    fMin = fSpecLin_a[0];
    iIndex = 0;
    for(ii=1; ii<iSpecSize; ii++) {
        if(fSpecLin_a[ii]<fMin) {
            fMin = fSpecLin_a[ii];
            iIndex = ii;
        }
    }
    fHold = fMin;
    fSpecLin_a[iIndex] = 1e10;
    fMin = fSpecLin_a[0];
    for(ii=1; ii<iSpecSize; ii++) {
        if(fSpecLin_a[ii]<fMin) {
            fMin = fSpecLin_a[ii];
        }
    }
    fSpecLin_a[iIndex] = fHold;
    // compare second minimum to noise power.
    // if noise power is lower, set to the second minimum.
    fNoiseCutoff = fNoiseTerm>fMin? fNoiseTerm : fMin;

    // limit search to either the main lobe points or 1/5 of fft size
    iMaxGapKill = floorf(min(1.0f + ((float) iSpecSize)/5.0f, WGCM_a->iMainLobePts));

    // Find where the clutter model falls below the noise floor
    // fModelPwr is only one side of the spectrum, when it falls below iGapOneSide
    // it tells us how wide we expect the clutter to be.
    for(iGapOneSide = 0; iGapOneSide < iMaxGapKill; iGapOneSide++) {// Don't use (iMaxGapKill - 1) to match behavior in C
        if(WGCM_a->fModelPwr[iGapOneSide] * fModelOff < fNoiseCutoff) {
            break;
        }
    }


    if(iGapOneSide > 0) {
        iGapPoints = (2 * iGapOneSide) - 1;

        // create array to replace clutter points
        fReplace = malloc(iGapPoints*sizeof(float));
        for(ii=0; ii<iGapPoints; ii++) fReplace[ii] = fNoiseTerm;

        fR0Data = 0.0f; // sum of power outside of gap
        fR0DGap = 0.0f; // sum of power in the gap
        cR1Data_r = 0.0f; cR1Data_i = 0.0f; // correlation terms for the first harmonic
        cR1DGap_r = 0.0f; cR1DGap_i = 0.0f;

        fGFrac =  ((float) iGapPoints) / ((float) iSpecSize);
        fSpill = ((float) (((50 - iSpecSize)>0)? (50 - iSpecSize) : 0)) / 50.0f; // ???
        iSignalPoints = 0; // bin count for normalization

        // loop outside of clutter gap (data region)
        fTerm = 0.0f;
        for(ii = iGapOneSide;  ii<(iSpecSize - iGapOneSide+1); ii++) {
            // sum data outside of clutter gap
            fTerm = fSpecLin_a[ii];
            fR0Data = fR0Data + fTerm;
            // accumulate first harmonic correlation
            cR1Data_r = cR1Data_r + fTerm * dftConf_a->fCorDots[ii]; // used for doppler ???
            cR1Data_i = cR1Data_i + fTerm * dftConf_a->fCorDots[iSpecSize+ii];
            iSignalPoints = iSignalPoints + 1;
        }
        fTerm = fSpill * fTerm + (1 - fSpill) * fNoiseTerm; // ???

        // loops inside of clutter gap - at the ends of the array
        for(ii = (iSpecSize - iGapOneSide + 1); ii<iSpecSize; ii++) {
            // sum data inside of clutter gap
            fR0DGap = fR0DGap + fTerm;
            // accumulate first harmonic correlation
            cR1DGap_r = cR1DGap_r + fTerm * dftConf_a->fCorDots[ii];
            cR1DGap_i = cR1DGap_i + fTerm * dftConf_a->fCorDots[iSpecSize+ii];
            iSignalPoints = iSignalPoints + 1;
            fTerm = fNoiseTerm;
        }
        fTerm = fSpill * fSpecLin_a[iGapOneSide] + (1 - fSpill) * fNoiseTerm;
        for(ii = (iGapOneSide - 1); ii>= 0; ii--) {
            // sum data inside of clutter gap
            fR0DGap = fR0DGap + fTerm;
            // accumulate first harmonic correlation
            cR1DGap_r = cR1DGap_r + fTerm * dftConf_a->fCorDots[ii];
            cR1DGap_i = cR1DGap_i + fTerm * dftConf_a->fCorDots[iSpecSize+ii];
            iSignalPoints = iSignalPoints + 1;
            fTerm = fNoiseTerm;
        }
        // subtract noise power so there is only clutter and signal power
        fR0Data = fR0Data - (fNoisePower_a * (1 - fGFrac));
        fR0DGap = fR0DGap - (fNoisePower_a * fGFrac);

        if(fR0Data > 0) {
            fR0MGap = fR0DGap;
            cR1MGap_r = cR1DGap_r; cR1MGap_i = cR1DGap_i;

            fSigmaSqMin = powf(PI / ((float) iSpecSize),2);

            // iterate to fit gaussian
            fR0Last =0.0f; fR1ArgLast = 0.0f;
            for(iPass = 0; iPass<GMAPMAXITERS; iPass++) {
                fR0 = fR0Data + fR0MGap; // total power - signal and clutter
                cR1_r = cR1Data_r + cR1MGap_r; cR1_i = cR1Data_i + cR1MGap_i;

                // compute mean doppler velocity
                // magnitude and phase of first harmonic
                fR1Mag = sqrtf(cR1_r*cR1_r+cR1_i*cR1_i);
                fR1Arg = atan2f(cR1_i, cR1_r);
                fMeanV = roundf(-fR1Arg * (32768.0f / PI));

                // estimate gaussian width
                if (fR0 > SIGMASQMAXRATIO * fR1Mag){
                    fSigmaSq = SIGMASQMAX;
                } else {
                    fSigmaSq = 2.0f * logf(fR0 / fR1Mag); // gaussian assumption
                    fSigmaSq = min(max(fSigmaSq, fSigmaSqMin), SIGMASQMAX);
                }
                fR0Scale = fR0 * sqrtf(2.0f*PI) / (sqrtf(fSigmaSq) * ((float) iSpecSize)); // normalize gaussian


                fR0MGap = 0;
                cR1MGap_r = 0; cR1MGap_i = 0;

                // fill replacement bins with modeled gaussian
                for(ii = 0; ii<iGapPoints; ii++) {
                    iIntVel = ii - (iGapOneSide - 1);
                    iTerm = (iIntVel + iSpecSize)%iSpecSize;
                    fVNorm = 2.0f * PI * ((float) iIntVel)/ ((float) iSpecSize);
                    fVOff = (floorf(fVNorm * (32768.0f / PI)) - fMeanV) * (PI / 32768.0f);
                    fModel = fR0Scale * expf(-(fVOff*fVOff) / (2.0f * fSigmaSq));

                    fReplace[ii] = fModel + fNoiseTerm;
                    fR0MGap = fR0MGap + fModel;

                    fModel = fModel + fNoiseTerm;

                    cR1MGap_r = cR1MGap_r + fModel * dftConf_a->fCorDots[iTerm];
                    cR1MGap_i = cR1MGap_i + fModel * dftConf_a->fCorDots[iSpecSize+iTerm];

                }

                // break from iteration if we are close enough
                if(iPass) {
                    if ((fabsf(fR1Arg - fR1ArgLast) < (VELPROGRESS * PI)) &&
                             (fR0 < (POWPROGRESS * fR0Last)) &&
                             (fR0Last < (POWPROGRESS * fR0)))
                        break;
                }
                // accumulate terms for next iteration
                fR0Last = fR0;
                fR1ArgLast = fR1Arg;
            }
        }

        // apply replacement from iteration
        fSpecSum = 0.0f;
        fPolySum = 0.0f;
        for (ii = (-iGapOneSide + 1); ii<iGapOneSide; ii++) {
            pTerm = fSpecLin_a[(ii + iSpecSize)%iSpecSize];
            // Interpolate.... or not!
            if (interp) {
                fNewVal = fReplace[ii + (iGapOneSide - 1)];
            } else {
                fNewVal = 0.0f;
            }
            fSpecSum = fSpecSum + pTerm;
            fPolySum = fPolySum + fNewVal;
            fSpecLin_a[(ii + iSpecSize)%iSpecSize] = fNewVal;
        }
        // calculate power removed
        if (fSpecSum > fPolySum) {
            fPowRemoved = fSpecSum - fPolySum;
        } else {
            fPowRemoved = 0.0f;
        }

        free(fReplace);
    } else {
        iGapPoints = 0;
        fPowRemoved = 0.0f;
    }

    // set outputs
    *piGapPoints = (float) iGapPoints;
    *pfPowRemoved = (float) fPowRemoved;
}
