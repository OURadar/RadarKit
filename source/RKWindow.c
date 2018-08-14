//
//  RKWindow.c
//  RadarKit
//
//  Created by Boon Leng Cheong on 3/18/15.
//  Copyright (c) 2015 Boon Leng Cheong. All rights reserved.
//

#include <RadarKit/RKWindow.h>
#include "bessi.c"

//
// Hann window
//
void hann(double *w, const int n) {
    int i, j;
    bool odd = n % 2;
    unsigned int m = (n + 1) / 2;
    double twid = 2.0 * M_PI / ((double)n + 1.0);

    if (n == 0) {
        return;
    }

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

//
// Hamming window
//
void hamming(double *w, const int n) {
    int i, j;
    bool odd = n % 2;
    unsigned int m = (n + 1) / 2;
    double twid = 2.0 * M_PI / ((double)n - 1.0);

    if (n == 0) {
        return;
    }

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

//
// Kaiser window
//
void kaiser(double *w, const int n, const double beta) {
    int i, j;
    bool odd = n % 2;
    unsigned int m = (n + 1) / 2;
    double bes = BESSI0(beta);
    double x_end = ((double)n - 1); x_end *= x_end;
    double *x, *v;
    
    if (n == 0) {
        return;
    }
    
    x = (double *)malloc((m + 1) * sizeof(double));
    v = (double *)malloc(m * sizeof(double));
    memset(x, 0, (m + 1) * sizeof(double));
    memset(v, 0, m * sizeof(double));
    
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
    free(x);
    free(v);
    return;
}

//
// Trapezoid window
// gamma to 1.0 when gamma > 0.0
// 1.0 to abs(gamma) when gamma < 0.0
//
void trapezoid(double *w, const int n, const double gamma) {
    int i;
    double slope = (gamma < 0.0 ? (fabs(gamma) - 1.0) : (1.0 - gamma)) / (n - 1);
    double offset = gamma < 0.0 ? 1.0 : gamma;
    for (i = 0; i < n; i++) {
        w[i] = slope * i + offset;
    }
}

void tukey(double *w, const int n, const double r) {
    int i, nl, nh;
    double p, t, dt;
    if (r <= 0.0) {
        for (i = 0; i < n; i++) {
            w[i] = 1.0;
        }
    } else if (r >= 1.0) {
        hann(w, n);
    } else {
        // Period of the taper portion is 0.5 * p
        p = 0.5 * r;
        nl = floor(p * (n - 1)) + 1;
        nh = n - nl;
        dt = 1.0 / (n - 1);
        t = 0.0;
        //printf("nl = %d   nh = %d  dt = %.3f\n", nl, nh, dt);
        for (i = 0; i < nl; i++) {
            w[i] = 0.5 * (1.0 + cos(M_PI / p * (t - p)));
            t += dt;
        }
        for (; i < nh; i++) {
            w[i] = 1.0;
            t += dt;
        }
        for (; i < n; i++) {
            w[i] = 0.5 * (1.0 + cos(M_PI / p * (t + p)));
            t += dt;
        }
    }
}

#pragma mark - Methods

//
// RKWindowMake(buffer, RKWindowTypeHann, 20) creates a Hann window of length 20
// RKWindowMake(buffer, RKWindowTypeKaiser, 20, 0.2) creates a Kaiser window of length 20 with factor 0.2
//
void RKWindowMake(RKFloat *buffer, RKWindowType type, const int length, ...) {
    int k;
    double param;
    va_list args;
    
    if (length == 0) {
        return;
    }
    
    double *window = (double *)malloc(length * sizeof(double));
    memset(window, 0, length * sizeof(double));

    va_start(args, length);

    switch (type) {
        
        case RKWindowTypeHann:
            hann(window, length);
            break;
        
        case RKWindowTypeHamming:
            hamming(window, length);
            break;
        
        case RKWindowTypeKaiser:
            param = va_arg(args, double);
            kaiser(window, length, param);
            break;
        
        case RKWindowTypeTrapezoid:
            param = va_arg(args, double);
            trapezoid(window, length, param);
            break;

        case RKWindowTypeTukey:
            param = va_arg(args, double);
            tukey(window, length, param);
            break;

        case RKWindowTypeBoxCar:
        default:
            for (k = 0; k < length; k++) {
                window[k] = 1.0;
            }
            break;
    }
    
    va_end(args);
    
    // Copy to RadarKit buffer
    for (k = 0; k < length; k++) {
        buffer[k] = (RKFloat)window[k];
    }
    
    free(window);
}
