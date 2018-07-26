UNAME := $(shell uname)
UNAME_M := $(shell uname -m)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

$(info UNAME_M = $(shell echo -e "\033[38;5;220m")${UNAME_M}$(shell echo -e "\033[0m"))
$(info GIT_BRANCH = $(shell echo -e "\033[38;5;46m")${GIT_BRANCH}$(shell echo -e "\033[0m"))

CFLAGS = -std=gnu99 -O2
ifeq ($(GIT_BRANCH), beta)
CFLAGS += -ggdb
endif

CFLAGS += -march=native -mfpmath=sse -Wall -Wno-unknown-pragmas
CFLAGS += -I headers -I /usr/local/include -I /usr/include -fPIC

ifeq ($(UNAME), Darwin)
CFLAGS += -fms-extensions -Wno-microsoft
endif

# Some heavy debuggning flags
#CFLAGS += -DDEBUG_IIR
#CFLAGS += -DDEBUG_IQ
#CFLAGS += -DDEBUG_FILE_MANAGER

LDFLAGS = -L ./ -L /usr/local/lib

OBJS = RadarKit.o RKRadar.o RKCommandCenter.o RKTest.o
OBJS += RKFoundation.o RKMisc.o RKDSP.o RKSIMD.o RKClock.o RKWindow.o
OBJS += RKPreference.o
OBJS += RKFileManager.o RKHostMonitor.o
OBJS += RKConfig.o RKHealth.o
OBJS += RKPulseCompression.o RKPulseRingFilter.o RKMoment.o
OBJS += RKRadarRelay.o
OBJS += RKNetwork.o RKServer.o RKClient.o
OBJS += RKPulsePair.o RKMultiLag.o
OBJS += RKPosition.o
OBJS += RKHealthRelayTweeta.o RKPedestalPedzy.o
OBJS += RKRawDataRecorder.o RKSweep.o RKSweepFile.o RKProduct.o RKProductFile.o RKHealthLogger.o
OBJS += RKWaveform.o

OBJS_PATH = objects
OBJS_WITH_PATH = $(addprefix $(OBJS_PATH)/, $(OBJS))

RKLIB = libradarkit.a

PROGS = rktest simple-emulator

ifeq ($(UNAME), Darwin)
# Mac OS X
CC = clang
CFLAGS += -D_DARWIN_C_SOURCE -Wno-deprecated-declarations -mmacosx-version-min=10.9
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

LDFLAGS += -lradarkit -lfftw3f -lnetcdf -lpthread -lz -lm

ifeq ($(UNAME), Darwin)
else
LDFLAGS += -lrt
endif

#all: $(RKLIB) install rktest
all: $(RKLIB) $(PROGS)

$(OBJS_PATH)/%.o: source/%.c | $(OBJS_PATH)
	$(CC) $(CFLAGS) -I headers/ -c $< -o $@

$(OBJS_PATH):
	mkdir -p $@

$(RKLIB): $(OBJS_WITH_PATH)
	ar rvcs $@ $(OBJS_WITH_PATH)

$(PROGS): %: %.c libradarkit.a
	@echo -e "\033[38;5;203m$@\033[0m"
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(PROGS) $(RKLIB) *.dSYM *.log
	rm $(OBJS_PATH)/*.o

install:
	sudo cp -rp headers/RadarKit headers/RadarKit.h /usr/local/include/
	sudo cp -p $(RKLIB) /usr/local/lib/

uninstall:
	rm -rf /usr/local/include/RadarKit.h /usr/local/include/RadarKit
	rm -rf /usr/local/lib/$(RKLIB)
