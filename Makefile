KERNEL := $(shell uname)
MACHINE := $(shell uname -m)
KERNEL_VER := $(shell uname -v)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
CPUS := $(shell (nproc --all || sysctl -n hw.ncpu) 2>/dev/null || echo 1)
VERSION := $(shell (grep __RKVersion__ headers/RadarKit/RKVersion.h | grep -oE '\".*\"' | sed 's/"//g'))

CFLAGS = -O2
ifneq ($(GIT_BRANCH), master)
	CFLAGS += -g -DBETA_BRANCH

	VERSION := $(VERSION)b
endif

# Some other heavy debuggning flags
# CFLAGS += -DDEBUG_IIR
# CFLAGS += -DDEBUG_IQ
# CFLAGS += -DDEBUG_FILE_MANAGER
# CFLAGS += -DDEBUG_WAVEFORM_NORMALIZATION
# CFLAGS += -D_SHOW_PRETTY_STRING_MEMORY
# CFLAGS += -DDEBUG_PULSE_ENGINE_WAIT
# CFLAGS += -DDEBUG_MUTEX_DESTROY
# CFLAGS += -DDEBUG_NAVEEN

CFLAGS += -std=c11
CFLAGS += -Wall
CFLAGS += -Woverlength-strings
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -fPIC

ifeq ($(HOMEBREW_PREFIX), )
	ifeq ($(MACHINE), arm64)
		HOMEBREW_PREFIX = /opt/homebrew
	else
		HOMEBREW_PREFIX = /usr/local
	endif
endif

ifeq ($(MACHINE), x86_64)
	CFLAGS += -march=native
	CFLAGS += -mfpmath=sse
endif

CFLAGS += -Iheaders -Iheaders/RadarKit
CFLAGS += -I${HOMEBREW_PREFIX}/include
CFLAGS += -I${HOMEBREW_PREFIX}/opt/openssl@1.1/include

LDFLAGS = -L${HOMEBREW_PREFIX}/lib
LDFLAGS += -L${HOMEBREW_PREFIX}/opt/openssl@1.1/lib

OBJS = RadarKit.o RKRadar.o RKCommandCenter.o RKReporter.o RKTest.o
OBJS += RKFoundation.o RKMisc.o RKDSP.o RKSIMD.o RKClock.o RKWindow.o RKRamp.o
OBJS += RKScratch.o
OBJS += RKPreference.o
OBJS += RKFileManager.o RKHostMonitor.o
OBJS += RKConfig.o
OBJS += RKWaveform.o RKFileHeader.o
OBJS += RKHealthEngine.o
OBJS += RKPositionEngine.o RKSteerEngine.o
OBJS += RKPulseEngine.o RKPulseRingFilter.o
OBJS += RKRadarRelay.o
OBJS += RKNetwork.o RKServer.o RKClient.o RKWebSocket.o
OBJS += RKMomentEngine.o RKPulsePair.o RKMultiLag.o RKSpectralMoment.o RKCalibrator.o
OBJS += RKHealthRelayTweeta.o RKHealthRelayNaveen.o RKPedestalPedzy.o
OBJS += RKRawDataRecorder.o RKSweepEngine.o RKSweepFile.o RKProduct.o RKProductFile.o RKHealthLogger.o

OBJS_PATH = objects
OBJS_WITH_PATH = $(addprefix $(OBJS_PATH)/, $(OBJS))

STATIC_LIB = libradarkit.a
SHARED_LIB = libradarkit.so

PROGS = rkutil simple-emulator pgen

ifeq ($(KERNEL), Darwin)
	# macOS
	CC = clang
	CFLAGS += -D_DARWIN_C_SOURCE -Wno-deprecated-declarations -fms-extensions -Wno-microsoft
else
	# Old Debian
	ifeq ($(MACHINE), i686)
		CFLAGS += -D_GNU_SOURCE -D_EXPLICIT_INTRINSIC -msse -msse2 -msse3 -msse4 -msse4.1
		LDFLAGS += -L/usr/lib64
	else
		CFLAGS += -D_GNU_SOURCE
		LDFLAGS += -L/usr/lib64 -L/usr/lib/x86_64-linux-gnu
	endif
endif

LDFLAGS += -lfftw3f -lnetcdf -lpthread -lz -lm -lssl

ifneq ($(KERNEL), Darwin)
	LDFLAGS += -lrt
endif

ifeq ($(shell echo "\033"), \033)
	EFLAG := -e
endif

all: showinfo $(STATIC_LIB) $(SHARED_LIB) $(PROGS)

showinfo:
	@echo $(EFLAG) "\
	KERNEL_VER = \033[38;5;15m$(KERNEL_VER)\033[m\n\
	KERNEL = \033[38;5;15m$(KERNEL)\033[m\n\
	MACHINE = \033[38;5;220m$(MACHINE)\033[m\n\
	VERSION = \033[38;5;46m$(VERSION)\033[m\n\
	GIT_BRANCH = \033[38;5;46m$(GIT_BRANCH)\033[m\n\
	HOMEBREW_PREFIX = \033[38;5;214m$(HOMEBREW_PREFIX)\033[m\n\
	EFLAG = \033[38;5;214m$(EFLAG)\033[m\n\
	CPUS = \033[38;5;203m$(CPUS)\033[m"


MAKEFLAGS += --jobs=$(CPUS)

$(OBJS_PATH)/%.o: source/%.c | $(OBJS_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_PATH):
	mkdir -p $@

$(STATIC_LIB): $(OBJS_WITH_PATH)
	@echo $(EFLAG) "\033[38;5;118m$@\033[m"
	ar rvcs $@ $(OBJS_WITH_PATH)

$(SHARED_LIB): $(OBJS_WITH_PATH)
	@echo $(EFLAG) "\033[38;5;118m$@\033[m"
	$(CC) -shared -o $@ $(OBJS_WITH_PATH) $(LDFLAGS)

$(PROGS): %: %.c $(STATIC_LIB)
	@echo $(EFLAG) "\033[38;5;45m$@\033[m"
	$(CC) $(CFLAGS) -o $@ $< $(STATIC_LIB) $(LDFLAGS)

clean:
	rm -f $(PROGS) $(STATIC_LIB) $(SHARED_LIB) *.log
	rm -f $(OBJS_PATH)/*.o
	rm -rf *.dSYM

install: showinfo
	cp -rp headers/RadarKit headers/RadarKit.h ${HOMEBREW_PREFIX}/include/
	cp -p $(STATIC_LIB) ${HOMEBREW_PREFIX}/lib/
	cp -p $(SHARED_LIB) ${HOMEBREW_PREFIX}/lib/$(SHARED_LIB).$(VERSION)
	[ -f ${HOMEBREW_PREFIX}/lib/$(SHARED_LIB) ] && rm -f ${HOMEBREW_PREFIX}/lib/$(SHARED_LIB) || :
	ln -s ${HOMEBREW_PREFIX}/lib/$(SHARED_LIB).$(VERSION) ${HOMEBREW_PREFIX}/lib/$(SHARED_LIB)

uninstall:
	rm -rf ${HOMEBREW_PREFIX}/include/RadarKit.h ${HOMEBREW_PREFIX}/include/RadarKit
	rm -rf ${HOMEBREW_PREFIX}/lib/$(STATIC_LIB) ${HOMEBREW_PREFIX}/lib/$(SHARED_LIB)*
