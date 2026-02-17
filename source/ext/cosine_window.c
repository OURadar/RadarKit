// create cosine window. From: https://github.com/sidneycadot/WindowFunctions/blob/master/README.md

void cosine_window(float *w, unsigned n, const float * coeff, unsigned ncoeff, bool sflag)
{
    // Generalized cosine window.
    //
    // Many window functions described in signal processing literature
    // can be written as linear combinations of cosines over the window length.
    //
    // Let 'x' be values going from 0 for the first element, and 2*pi for the last element.
    //
    // The window can then be written as:
    //
    // w = c0 * cos(0 * x) + c1 * cos(1 * x) + c2 * cos(2 * x) + c3 * cos(3 * x) + ...
    //
    // (Note that the first term simplifies to just the constant value c0.)
    //
    // Examples of cosine windows implemented in Matlab:
    //
    //                              c0          c1           c2           c3            c4
    // -------------------------------------------------------------------------------------------
    // rectangular window          1.0
    // hann window                 0.5         -0.5
    // hamming window              0.54        -0.46
    // blackman window             0.42        -0.5         0.08
    // blackman-harris window      0.35875     -0.48829     0.14128      -0.01168
    // nuttall window              0.3635819   -0.4891775   0.1365995    -0.0106411
    // flattop window              0.21557895  -0.41663158  0.277263158  -0.083578947  0.006947368
    //
    // The "flattop" coefficients given above follow Matlab's "flattopwin" implementation.
    // The signal processing literature in fact describes many different 'flattop' windows.
    //
    // Note 1 : Octave defines the flattopwin coefficients differently, see implementation below.
    //
    //          The coefficient values used correspond to:
    //
    //          [0.21550795224343777, -0.4159303478298349, 0.2780052583940347, -0.08361708547045386, 0.006939356062238697]
    //
    // Note 2 : Octave defines the nuttallwin coefficients differently, see implementation below:
    //
    //          The coefficient values used are:
    //
    //          [0.355768, -0.487396, 0.144232, -0.012604]

	int i, j;

	const int count = sflag ? (n - 1) : n;

	if (n == 1) {
        // Special case for n == 1.
        w[0] = 1.0f;
    } else {
		w[0] = 0.0f;
        for (i = 0; i < n; ++i) {
            float v = 0.0f;
            for (j = 0; j < ncoeff; ++j) {
                v += coeff[j] * cosf(i * j * 2.0f * M_PI / count);
            }
            w[i] = v;
        }
    }
}
