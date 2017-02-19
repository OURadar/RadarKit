//
//  RKWindow.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKWindow.h>
#include "bessi.c"

void hann(double *w, const int n) {
    int i, j;
    unsigned int m = (n + 1) / 2;
    unsigned int odd = n % 2;
    double twid = 2.0 * M_PI / ((double)n + 1.0);
    for (i = 0; i < m; i++) {
        w[i] = 0.5 * (1.0 - cos(twid * ((double)i + 1.0)));
        //printf("%.7f\n", v[i]);
    }
    if (odd) {
        j = m - 1;
    } else {
        j = m;
    }
    while (j > 0) {
        j--;
        w[i++] = w[j];
    }
    return;
}

void hamming(double *w, const int n) {
    int i, j;
    unsigned int m = (n + 1) / 2;
    unsigned int odd = n % 2;
    double twid = 2.0 * M_PI / ((double)n - 1.0);
    for (i = 0; i < m; i++) {
        w[i] = 0.54 - 0.46 * cos(twid * ((double)i));
    }
    if (odd) {
        j = m - 1;
    } else {
        j = m;
    }
    while (j > 0) {
        j--;
        w[i++] = w[j];
    }
    return;
}

void kaiser(double *w, const int n, const double beta) {
    int i, j;
    unsigned int m = (n + 1) / 2;
    unsigned int odd = n % 2;
    double bes = BESSI0(beta);
    double x_end = ((double)n - 1); x_end *= x_end;
    double x[m], v[m];
    
    if (bes < 0.0) {
        bes -= bes;
    }
    
    if (odd) {
        for (i = 0; i < m; i++) {
            x[i] = (double)i;
        }
    } else {
        for (i = 0; i < m + 1; i++) {
            x[i] = (double)i + 0.5;
        }
    }
    
    for (i = 0; i < m; i++) {
        x[i] *= x[i] * 4.0;
        v[i] = BESSI0(beta * sqrt(1.0 - (x[i] / x_end))) / bes;
        //printf("x = %10.7f  v = %10.7f\n", x[i], v[i]);
    }
    
    i = m;
    j = 0;
    while (i > 1) {
        i--;
        w[j++] = v[i];
    }
    if (!odd) {
        i--;
        w[j++] = v[i];
    }
    i = 0;
    while (j < n) {
        w[j++] = v[i++];
    }
    return;
}

void RKWindowMake(RKFloat *buffer, RKWindowType type, const int length, ...) {
    int k;
    double beta;
    va_list args;
    
    double *window = (double *)malloc(length * sizeof(double));

    va_start(args, length);

    switch (type) {
        case RKWindowTypeHann:
            hann(window, length);
            break;
        case RKWindowTypeHamming:
            hamming(window, length);
            break;
        case RKWindowTypeKaiser:
            beta = va_arg(args, double);
            printf("beta = %.4f\n", beta);
            kaiser(window, length, beta);
            break;
        case RKWindowTypeBoxCar:
        default:
            for (k = 0; k < length; k++) {
                buffer[k] = (RKFloat)1.0;
            }
            break;
    }
    
    va_end(args);
    
    // Down sample to RadarKit precision
    for (k = 0; k < length; k++) {
        buffer[k] = (RKFloat)window[k];
    }
    
    free(window);
}
