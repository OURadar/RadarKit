UNAME := $(shell uname)

#CFLAGS = -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -I /usr/local/include -I /usr/include -fPIC -msse -msse2 -msse3 -mavx
#CFLAGS = -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC -msse -msse2 -msse3 -mavx
CFLAGS = -ggdb -std=gnu99 -march=native -mfpmath=sse -Os -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC
#CFLAGS = -std=gnu99 -march=native -mfpmath=sse -Os -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC
#CFLAGS += -fms-extensions -Wno-microsoft
LDFLAGS = -L /usr/local/lib
OBJS = RadarKit.o RKRadar.o RKCommandCenter.o RKTest.o RKSweep.o
OBJS += RKFoundation.o RKMisc.o RKDSP.o RKSIMD.o RKClock.o RKWindow.o
OBJS += RKPreference.o
OBJS += RKConfig.o RKPulseCompression.o RKMoment.o
OBJS += RKNetwork.o RKServer.o RKClient.o
OBJS += RKPulsePair.o RKMultiLag.o
OBJS += RKPosition.o RKPedestalPedzy.o
OBJS += RKHealth.o RKHealthRelayTweeta.o
OBJS += RKDataRecorder.o RKFileManager.o
OBJS += RKWaveform.o
RKLIB = libRadarKit.a

#CFLAGS += -DDEBUG_IQ
#CFLAGS += -mavx2 -mavx512cd -mavx512er -mavx512f -mavx512pf
#CFLAGS += -mavx2 -mavx512f
#CFLAGS += -mavx2

ifeq ($(UNAME), Darwin)
# Mac OS X
CC = clang
CFLAGS += -D_DARWIN_C_SOURCE -Wno-deprecated-declarations
else
CFLAGS += -D_GNU_SOURCE
LDFLAGS += -L /usr/lib64
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
