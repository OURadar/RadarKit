RadarKit
========

A toolkit with various components of a radar signal processor. Mainly implement the real-time operation of data collection, data transportation through network, rudimentary processing from raw I/Q data to moment data including multi-core pulse match filtering (compression). More comments will come later ...

Getting netcdf
CentOS 7

```shell
yum install epel-release
yum install netcdf
```
Mac OS X
```shell
brew install netcdf
```


Radar Struct
------------
This is about the only structure you need to worry about. A radar structure represents an object-like structure where everything is encapsulated.


### Life Cycle ###

These are functions that allocate and deallocate a radar struct.

```c
RKRadar *RKInitWithDesc(RKRadarDesc);
RKRadar *RKInitLean(void);               // For a lean system, PX-1000 like
RKRadar *RKInitMean(void);               // For a medium system, RaXPol like
RKRadar *RKInitFull(void);               // For a high-performance system, PX-10,000 like
RKRadar *RKInit(void);                   // Everything based on default settings, in between mean & lean
int RKFree(RKRadar *radar);
```


### Properties ###

Hardware hooks are provided to communicate with a digital transceiver, a positioner and various sensors. They must obey the protocol to implement three important functions: _init_, _exec_ and _free_ routines. These functions will be called to start the hardware routine, execute text form commands that will be passed down the master controller, and to deallocate the resources properly upon exit, respectively.

```c
// Set the transceiver. Pass in function pointers: init, exec and free
int RKSetTransceiver(RKRadar *radar,
                     void *initInput,
                     RKTransceiver initRoutine(RKRadar *, void *),
                     int execRoutine(RKTransceiver, const char *),
                     int freeRoutine(RKTransceiver));

// Set the pedestal. Pass in function pointers: init, exec and free
int RKSetPedestal(RKRadar *radar,
                  void *initInput,
                  RKPedestal initRoutine(RKRadar *, void *),
                  int execRoutine(RKPedestal, const char *),
                  int freeRoutine(RKPedestal));

// Some states of the radar
int RKSetVerbose(RKRadar *radar, const int verbose);
int RKSetProcessingCoreCounts(RKRadar *radar,
                              const unsigned int pulseCompressionCoreCount,
                              const unsigned int momentProcessorCoreCount);

// Some operating parameters
int RKSetWaveform(RKRadar *radar, const char *filename, const int group, const int maxDataLength);
int RKSetWaveformToImpulse(RKRadar *radar);
int RKSetWaveformTo121(RKRadar *radar);
uint32_t RKGetPulseCapacity(RKRadar *radar);
```

### Interactions ###

```c
// The radar engine state
int RKGoLive(RKRadar *);
int RKWaitWhileActive(RKRadar *);
int RKStop(RKRadar *);

// Positions
RKPosition *RKGetVacantPosition(RKRadar *);
void RKSetPositionReady(RKRadar *, RKPosition *);

// Pulses
RKPulse *RKGetVacantPulse(RKRadar *);
void RKSetPulseHasData(RKRadar *, RKPulse *);
void RKSetPulseReady(RKRadar *, RKPulse *);

// Rays
RKRay *RKGetVacantRay(RKRadar *);
void RKSetRayReady(RKRadar *, RKRay *);
```


RadarKit Test Program
---------------------

A test program is provided to assess if everything can run properly with your system.

```
rktest [options]

OPTIONS:
     Unless specifically stated, all options are interpreted in sequence. Some
     options can be specified multiples times for repetitions. For example, the
     verbosity level increaes by one for every -v.

  -c (--core) P,M (no space after comma)
         Sets the number of threads for pulse compression to P
         and the number of threads for product generator to M.
         If not specified, the default core counts are 8 / 4.

  -f (--prf) value
         Sets the pulse repetition frequency (PRF) to value in Hz.
         If not specified, the default PRF = 5000 Hz.

  -F (--fs or -b) value
         Sets the sampling frequency (bandwidth) to value in Hz.
         If not specified, the default will be used.

  -g (--gate) value
         Sets the number of range gates to value.
         If not specified, the default gate count is 8192.

  -h (--help)
         Shows this help text.

  -L (--test-lean-system)
         Run with arguments '-v -f 2000 -F 5e6 -c 2,2'.

  -M (--test-medium-system)
         Run with arguments '-v -f 5000 -F 20e6 -c 4,2'.

  -p (--pedzy-host) hostname
         Sets the host of pedzy pedestal controller.

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

  --test-processor
         Sets the program to test the moment processor.


EXAMPLES:
     Here are some examples of typical configurations.

  radar
         Runs the program with default settings.

  radar -f 2000
         Runs the program with PRF = 2000 Hz.
```
