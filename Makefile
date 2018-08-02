KERNEL := $(shell uname)
MACHINE := $(shell uname -m)
#KERNEL_VER := $(shell expr "$(shell uname -v)" : ".*-\([A-Za-z]*\)\ .*")
KERNEL_VER := $(shell uname -v)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

CFLAGS = -std=gnu99 -O2
ifneq ($(GIT_BRANCH), master)
	CFLAGS += -ggdb -DBETA_BRANCH
endif

# Some other heavy debuggning flags
#CFLAGS += -DDEBUG_IIR
#CFLAGS += -DDEBUG_IQ
#CFLAGS += -DDEBUG_FILE_MANAGER

CFLAGS += -march=native -mfpmath=sse -Wall -Wno-unknown-pragmas
CFLAGS += -I headers -I /usr/local/include -I /usr/include -fPIC

ifeq ($(KERNEL), Darwin)
	CFLAGS += -fms-extensions -Wno-microsoft
endif

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

# The command echo from macOS and Ubuntu needs no -e
ECHO_FLAG = -e
ifneq (, $(findstring Darwin, $(KERNEL_VER)))
	ECHO_FLAG =
endif
ifneq (, $(findstring Ubuntu, $(KERNEL_VER)))
	ECHO_FLAG =
endif

ifeq ($(KERNEL), Darwin)
	# Mac OS X
	CC = clang
	CFLAGS += -D_DARWIN_C_SOURCE -Wno-deprecated-declarations -mmacosx-version-min=10.9
else
	# Old Debian
	ifeq ($(MACHINE), i686)
		CFLAGS += -D_GNU_SOURCE -D_EXPLICIT_INTRINSIC -msse -msse2 -msse3 -msse4 -msse4.1
		LDFLAGS += -L /usr/lib64
	else
		CFLAGS += -D_GNU_SOURCE
		LDFLAGS += -L /usr/lib64
	endif
endif

LDFLAGS += -lradarkit -lfftw3f -lnetcdf -lpthread -lz -lm

ifeq ($(KERNEL), Darwin)
else
LDFLAGS += -lrt
endif

#all: $(RKLIB) install rktest
all: showinfo $(RKLIB) $(PROGS)

showinfo:
	@echo $(ECHO_FLAG) "KERNEL_VER = \033[38;5;15m$(KERNEL_VER)\033[0m"
	@echo $(ECHO_FLAG) "KERNEL = \033[38;5;15m$(KERNEL)\033[0m"
	@echo $(ECHO_FLAG) "MACHINE = \033[38;5;220m$(MACHINE)\033[0m"
	@echo $(ECHO_FLAG) "GIT_BRANCH = \033[38;5;46m$(GIT_BRANCH)\033[0m"

$(OBJS_PATH)/%.o: source/%.c | $(OBJS_PATH)
	$(CC) $(CFLAGS) -I headers/ -c $< -o $@

$(OBJS_PATH):
	mkdir -p $@

$(RKLIB): $(OBJS_WITH_PATH)
	ar rvcs $@ $(OBJS_WITH_PATH)

$(PROGS): %: %.c libradarkit.a
ifeq ($(KERNEL), Darwin)
	@echo "\033[38;5;203m$@\033[0m"
else
	@echo -e "\033[38;5;203m$@\033[0m"
endif
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(PROGS) $(RKLIB) *.log
	rm -f $(OBJS_PATH)/*.o
	rm -rf *.dSYM

install:
	sudo cp -rp headers/RadarKit headers/RadarKit.h /usr/local/include/
	sudo cp -p $(RKLIB) /usr/local/lib/

uninstall:
	rm -rf /usr/local/include/RadarKit.h /usr/local/include/RadarKit
	rm -rf /usr/local/lib/$(RKLIB)
