UNAME := $(shell uname)
UNAME_M := $(shell uname -m)

$(info $$UNAME_M = [${UNAME_M}])

CFLAGS =      -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC -msse -msse2 -msse3 -mavx
#CFLAGS = -ggdb -std=gnu99 -O2 -march=native -mfpmath=sse -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC
#CFLAGS =      -std=gnu99 -Os -march=native -mfpmath=sse -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC
CFLAGS += -fms-extensions -Wno-microsoft

LDFLAGS = -L /usr/local/lib

OBJS = RadarKit.o RKRadar.o RKCommandCenter.o RKTest.o
OBJS += RKFoundation.o RKMisc.o RKDSP.o RKSIMD.o RKClock.o RKWindow.o
OBJS += RKPreference.o
OBJS += RKFileManager.o RKHostMonitor.o
OBJS += RKConfig.o RKHealth.o RKPulseCompression.o RKMoment.o RKRadarRelay.o
OBJS += RKNetwork.o RKServer.o RKClient.o
OBJS += RKPulsePair.o RKMultiLag.o
OBJS += RKPosition.o
OBJS += RKHealthRelayTweeta.o RKPedestalPedzy.o
OBJS += RKDataRecorder.o RKSweep.o RKHealthLogger.o
OBJS += RKWaveform.o
RKLIB = libRadarKit.a

#CFLAGS += -DDEBUG_IQ
#CFLAGS += -mavx2 -mavx512cd -mavx512er -mavx512f -mavx512pf
#CFLAGS += -mavx2 -mavx512f
CFLAGS += -mavx2

ifeq ($(UNAME), Darwin)
# Mac OS X
CC = clang
CFLAGS += -D_DARWIN_C_SOURCE -Wno-deprecated-declarations
else
# Old Debian
ifeq ($(UNAME_M), i686)
CFLAGS += -D_GNU_SOURCE -D_EXPLICIT_INTRINSIC -msse -msse2 -msse3 -msse4 -msse4.1
LDFLAGS += -L /usr/lib64
else
CFLAGS += -D_GNU_SOURCE
LDFLAGS += -L /usr/lib64
endif
endif

LDFLAGS += -lfftw3f -lnetcdf -lpthread -lz -lm

ifeq ($(UNAME), Darwin)
else
LDFLAGS += -lrt
endif

#all: $(RKLIB) install rktest
all: $(RKLIB) rktest

$(OBJS): %.o: source/%.c
	$(CC) $(CFLAGS) -I headers/ -c $< -o $@

$(RKLIB): $(OBJS)
	ar rvcs $@ $(OBJS)

#rktest: RadarKitTest/main.c /usr/local/lib/libRadarKit.a
#	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)
rktest: RadarKitTest/main.c libRadarKit.a
	$(CC) -o $@ $(CFLAGS) $< $(OBJS) $(LDFLAGS)

clean:
	rm -f $(RKLIB)
	rm $(OBJS)

install:
	sudo cp -rp headers/RadarKit headers/RadarKit.h /usr/local/include/
	sudo cp -p libRadarKit.a /usr/local/lib/

uninstall:
	rm -rf /usr/local/include/RadarKit.h /usr/local/include/RadarKit
	rm -rf /usr/local/lib/libRadarKit.a
