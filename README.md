RadarKit
========

A toolkit with various components of a radar signal processor. Mainly implement the real-time operation of data collection, data transportation through network, rudimentary processing from raw I/Q data to moment data including multi-core pulse match filtering (compression). More comments will come later ...

```
RadarKit Test Program

rktest [options]

OPTIONS:
     Unless specifically stated, all options are interpreted in sequence. Some
     options can be specified multiples times for repetitions. For example, the
     debris particle count is set for each type sequentially by repeating the
     option multiple times for each debris type.

  -c (--core) P,M (no space after comma)
         Sets the number of threads for pulse compression to P
         and the number of threads for product generator to M.
         If not specified, the default core counts are 8 / 4.

  -f (--prf) value
         Sets the pulse repetition frequency (PRF) to value in Hz.
         If not specified, the default PRF = 5000 Hz.

  -F (--fs) value
         Sets the sampling frequency to value in Hz.
         If not specified, the default fs = 5e6 Hz = 5 MHz.

  -g (--gate) value
         Sets the number range gates to value.
         If not specified, the default PRF is 8192 Hz.

  -h (--help)
         Shows this help text.

  -L (--test-lean-system)
         Run with arguments '-v -f 2000 -F 5e6 -c 2,2'.

  -M (--test-medium-system)
         Run with arguments '-v -f 5000 -F 20e6 -c 4,2'.

  -p (--pedzy-host) hostname
         Sets the host of pedzy pedestal controller.

  -q (--quiet)
         Decreases verbosity level, which can be specified multiple times.

  -s (--simulate)
         Sets the program to simulate data stream (default, if none of the tests
         is specified).

  -v (--verbose)
         Increases verbosity level, which can be specified multiple times.

  --test-mod
         Sets the program to test modulo macros.

  --test-simd
         Sets the program to test SIMD instructions.
         To test the SIMD performance, use --test-simd=2

  --test-pulse-compression
         Sets the program to test the pulse compression using a simple case with.
         an impulse filter.


EXAMPLES:
     Here are some examples of typical configurations.

  radar
         Runs the program with default settings.

  radar -f 2000
         Runs the program with PRF = 2000 Hz.
```
