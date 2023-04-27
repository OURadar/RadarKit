KERNEL := $(shell uname)
MACHINE := $(shell uname -m)
KERNEL_VER := $(shell uname -v)
GIT_BRANCH := $(shell git rev-parse --abbrev-ref HEAD)

ifneq ($(GIT_BRANCH), master)
	CFLAGS += -ggdb -DBETA_BRANCH
endif

# Some other heavy debuggning flags
#CFLAGS += -DDEBUG_IIR
#CFLAGS += -DDEBUG_IQ
#CFLAGS += -DDEBUG_FILE_MANAGER
#CFLAGS += -DDEBUG_WAVEFORM_NORMALIZATION
#CFLAGS += -D_SHOW_PRETTY_STRING_MEMORY

CFLAGS += -O2
CFLAGS += -std=c11
CFLAGS += -Wall
# CFLAGS += -Wextra
# CFLAGS += -Wpedantic
CFLAGS += -Woverlength-strings
CFLAGS += -Wno-unknown-pragmas

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
	# CFLAGS += -mfma
endif

CFLAGS += -Iheaders -Iheaders/RadarKit -fPIC

LDFLAGS = -L./

ifeq ($(KERNEL), Darwin)
	CFLAGS += -I${HOMEBREW_PREFIX}/include
	CFLAGS += -I${HOMEBREW_PREFIX}/opt/openssl@1.1/include

	LDFLAGS += -L${HOMEBREW_PREFIX}/lib
	LDFLAGS += -L${HOMEBREW_PREFIX}/opt/openssl@1.1/lib
endif

OBJS = RadarKit.o RKRadar.o RKCommandCenter.o RKReporter.o RKTest.o
OBJS += RKFoundation.o RKMisc.o RKDSP.o RKSIMD.o RKClock.o RKWindow.o RKRamp.o
OBJS += RKMoment.o
OBJS += RKPreference.o
OBJS += RKFileManager.o RKHostMonitor.o
OBJS += RKConfig.o
OBJS += RKWaveform.o
OBJS += RKHealthEngine.o
OBJS += RKPositionEngine.o RKSteerEngine.o
OBJS += RKPulseEngine.o RKPulseRingFilter.o
OBJS += RKRadarRelay.o
OBJS += RKNetwork.o RKServer.o RKClient.o RKWebSocket.o
OBJS += RKMomentEngine.o RKPulsePair.o RKMultiLag.o RKSpectralMoment.o RKCalibrator.o
OBJS += RKHealthRelayTweeta.o RKPedestalPedzy.o
OBJS += RKRawDataRecorder.o RKSweepEngine.o RKSweepFile.o RKProduct.o RKProductFile.o RKHealthLogger.o

OBJS_PATH = objects
OBJS_WITH_PATH = $(addprefix $(OBJS_PATH)/, $(OBJS))

RKLIB = libradarkit.a

# PROGS = rkutil simple-emulator rknchead pgen owav2wav
PROGS = rkutil simple-emulator pgen

# The command echo from macOS and Ubuntu needs no -e
ECHO_FLAG = -e
ifneq (, $(findstring Darwin, $(KERNEL_VER)))
	ECHO_FLAG =
endif
ifneq (, $(findstring Linux, $(KERNEL_VER)))
	ECHO_FLAG =
endif

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

LDFLAGS += -lradarkit -lfftw3f -lnetcdf -lpthread -lz -lm -lssl -lcrypto

ifeq ($(KERNEL), Darwin)
else
	LDFLAGS += -lrt
endif

all: showinfo $(RKLIB) $(PROGS)

showinfo:
	@echo $(ECHO_FLAG) "KERNEL_VER = \033[38;5;15m$(KERNEL_VER)\033[m"
	@echo $(ECHO_FLAG) "KERNEL = \033[38;5;15m$(KERNEL)\033[m"
	@echo $(ECHO_FLAG) "MACHINE = \033[38;5;220m$(MACHINE)\033[m"
	@echo $(ECHO_FLAG) "GIT_BRANCH = \033[38;5;46m$(GIT_BRANCH)\033[m"
	@echo $(ECHO_FLAG) "HOMEBREW_PREFIX = \033[38;5;214m$(HOMEBREW_PREFIX)\033[m"

$(OBJS_PATH)/%.o: source/%.c | $(OBJS_PATH)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_PATH):
	mkdir -p $@

$(RKLIB): $(OBJS_WITH_PATH)
	ar rvcs $@ $(OBJS_WITH_PATH)

$(PROGS): %: %.c libradarkit.a
ifeq ($(KERNEL), Darwin)
	@echo "\033[38;5;45m$@\033[m"
else
	@echo $(ECHO_FLAG) "\033[38;5;45m$@\033[m"
endif
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(PROGS) $(RKLIB) *.log
	rm -f $(OBJS_PATH)/*.o
	rm -rf *.dSYM

install: showinfo
	cp -rp headers/RadarKit headers/RadarKit.h ${HOMEBREW_PREFIX}/include/
	cp -p $(RKLIB) ${HOMEBREW_PREFIX}/lib/

uninstall:
	rm -rf ${HOMEBREW_PREFIX}/include/RadarKit.h ${HOMEBREW_PREFIX}/include/RadarKit
	rm -rf ${HOMEBREW_PREFIX}/lib/$(RKLIB)
