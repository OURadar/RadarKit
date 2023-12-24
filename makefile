KERNEL := $(shell uname)
MACHINE := $(shell uname -m)
KERNEL_VER := $(shell uname -v)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)
CPUS := $(shell (nproc --all || sysctl -n hw.ncpu) 2>/dev/null || echo 1)
VERSION := $(shell (grep __RKVersion__ headers/RadarKit/RKVersion.h | grep -oE '\".*\"' | sed 's/"//g'))

CFLAGS = -O2
# ifneq ($(GIT_BRANCH), master)
# 	CFLAGS += -g -DBETA_BRANCH

# 	VERSION := $(VERSION)b
# endif

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

ifeq ($(PREFIX), )
	ifeq ($(MACHINE), arm64)
		PREFIX = /opt/homebrew
	else
		PREFIX = /usr/local
	endif
endif

ifeq ($(MACHINE), x86_64)
	CFLAGS += -march=native
	CFLAGS += -mfpmath=sse
endif

CFLAGS += -Iheaders -Iheaders/RadarKit
CFLAGS += -I${PREFIX}/include
CFLAGS += -I${PREFIX}/opt/openssl@1.1/include

LDFLAGS = -L${PREFIX}/lib
LDFLAGS += -L${PREFIX}/opt/openssl@1.1/lib

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
OBJS += RKMomentEngine.o RKPulsePair.o RKMultiLag.o RKPulseATSR.o RKSpectralMoment.o RKCalibrator.o RKNoiseEstimator.o
OBJS += RKHealthRelayTweeta.o RKHealthRelayNaveen.o RKPedestalPedzy.o
OBJS += RKRawDataRecorder.o RKSweepEngine.o RKSweepFile.o RKProduct.o RKProductFile.o RKHealthLogger.o

OBJS_OUT_PATH := objects
OBJS_SRC_PATH := source
OBJS_SRC := $(wildcard $(OBJS_SRC_PATH)/*.c)
OBJS := $(patsubst $(OBJS_SRC_PATH)/%.c,$(OBJS_OUT_PATH)/%.o,$(OBJS_SRC))

EXAMPLE_OUT_PATH := build
EXAMPLE_SRC_PATH := examples
EXAMPLE_SRC := $(wildcard $(EXAMPLE_SRC_PATH)/*.c)
EXAMPLES := $(patsubst $(EXAMPLE_SRC_PATH)/%.c,$(EXAMPLE_OUT_PATH)/%,$(EXAMPLE_SRC))

CTYPES_OUT_PATH := python/src/radarkit

STATIC_LIB = libradarkit.a
SHARED_LIB = libradarkit.so

PROGS = rkutil

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

all: showinfo $(STATIC_LIB) $(SHARED_LIB) $(PROGS) $(EXAMPLES)

showinfo:
	@echo $(EFLAG) "\
	KERNEL_VER = \033[38;5;15m$(KERNEL_VER)\033[m\n\
	KERNEL = \033[38;5;15m$(KERNEL)\033[m\n\
	MACHINE = \033[38;5;87m$(MACHINE)\033[m\n\
	VERSION = \033[38;5;46m$(VERSION)\033[m\n\
	GIT_BRANCH = \033[38;5;226m$(GIT_BRANCH)\033[m\n\
	PREFIX = \033[38;5;214m$(PREFIX)\033[m\n\
	EFLAG = \033[38;5;208m$(EFLAG)\033[m\n\
	CPUS = \033[38;5;203m$(CPUS)\033[m\n\
	OBJS = \033[38;5;213m$(OBJS)\033[m\n\
	EXAMPLES = \033[38;5;45m$(EXAMPLES)\033[m\n\
	"

# IMPORTANT: KEEP THOSE SPACES BEFORE THE SLASHES
ctypes: $(SHARED_LIB) | $(CTYPES_OUT_PATH)
	ctypesgen -I$(PREFIX)/include -Iheaders -Iheaders/RadarKit -L./ -lradarkit \
	headers/RadarKit/RKTypes.h \
	headers/RadarKit/RKMisc.h \
	headers/RadarKit/RKFoundation.h \
	headers/RadarKit/RKConfig.h \
	headers/RadarKit/RKDSP.h \
	headers/RadarKit/RKPulseEngine.h \
	headers/RadarKit/RKFileHeader.h \
	headers/RadarKit/RKScratch.h \
	headers/RadarKit/RKRawDataRecorder.h \
	headers/RadarKit/RKMomentEngine.h \
	headers/RadarKit/RKNoiseEstimator.h \
	headers/RadarKit/RKSweepEngine.h \
	headers/RadarKit/RKPulseRingFilter.h \
	headers/RadarKit/RKMultiLag.h \
	headers/RadarKit/RKPulseATSR.h \
	headers/RadarKit/RKSpectralMoment.h \
	headers/RadarKit/RKPulsePair.h \
	headers/RadarKit/RKWaveform.h \
	headers/RadarKit.h \
	headers/RadarKit/RKTest.h \
	-o $(CTYPES_OUT_PATH)/_ctypes_.py

MAKEFLAGS += --jobs=$(CPUS)

$(EXAMPLE_OUT_PATH) $(OBJS_OUT_PATH) $(CTYPES_OUT_PATH):
	mkdir -p $@

$(OBJS_OUT_PATH)/%.o: $(OBJS_SRC_PATH)/%.c | $(OBJS_OUT_PATH)
	@echo $(EFLAG) "\033[38;5;213m$@\033[m $^"
	$(CC) $(CFLAGS) -I headers/ -c $< -o $@

$(STATIC_LIB): $(OBJS)
	@echo $(EFLAG) "\033[38;5;118m$@\033[m"
	ar rvcs $@ $(OBJS)

$(SHARED_LIB): $(OBJS)
	@echo $(EFLAG) "\033[38;5;118m$@\033[m"
	$(CC) -shared -o $@ $(OBJS) $(LDFLAGS)

$(EXAMPLE_OUT_PATH)/%: $(EXAMPLE_SRC_PATH)/%.c $(STATIC_LIB) | $(EXAMPLE_OUT_PATH)
	@echo $(EFLAG) "\033[38;5;45m$@\033[m"
	$(CC) $(CFLAGS) -o $@ $< $(STATIC_LIB) $(LDFLAGS)

$(PROGS): %: %.c $(STATIC_LIB)
	@echo $(EFLAG) "\033[38;5;45m$@\033[m"
	$(CC) $(CFLAGS) -o $@ $< $(STATIC_LIB) $(LDFLAGS)

clean:
	rm -f $(PROGS) $(STATIC_LIB) $(SHARED_LIB) *.log
	rm -rf $(EXAMPLE_OUT_PATH) $(OBJS_OUT_PATH)
	rm -rf *.dSYM

cleanctypes:
	rm -f $(CTYPES_OUT_PATH/_ctypes_.py

install: showinfo
	cp -rp headers/RadarKit headers/RadarKit.h ${PREFIX}/include/
	cp -p $(STATIC_LIB) ${PREFIX}/lib/
	cp -p $(SHARED_LIB) ${PREFIX}/lib/$(SHARED_LIB).$(VERSION)
	[ -f ${PREFIX}/lib/$(SHARED_LIB) ] && rm -f ${PREFIX}/lib/$(SHARED_LIB) || :
	ln -s ${PREFIX}/lib/$(SHARED_LIB).$(VERSION) ${PREFIX}/lib/$(SHARED_LIB)

uninstall:
	rm -rf ${PREFIX}/include/RadarKit.h ${PREFIX}/include/RadarKit
	rm -rf ${PREFIX}/lib/$(STATIC_LIB) ${PREFIX}/lib/$(SHARED_LIB)*
