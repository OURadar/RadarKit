UNAME := $(shell uname)

#CFLAGS = -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -I /usr/local/include -I /usr/include -fPIC -msse -msse2 -msse3 -mavx
#CFLAGS = -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC -msse -msse2 -msse3 -mavx
CFLAGS = -std=gnu99 -march=native -mfpmath=sse -Os -Wall -Wno-unknown-pragmas -I headers -I /usr/local/include -I /usr/include -fPIC
LDFLAGS = -L /usr/local/lib
OBJS = RadarKit.o RKRadar.o RKFoundation.o RKMisc.o RKPulseCompression.o RKServer.o RKLocalCommandCenter.o RKSIMD.o RKMoment.o RKTest.o RKPulsePair.o
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
LDFLAGS += -lRadarKit -lfftw3f -lpthread -lm -lrt

all: $(RKLIB) install radar

$(OBJS): %.o: source/%.c
	$(CC) $(CFLAGS) -I headers/ -c $< -o $@

$(RKLIB): $(OBJS)
	ar rvcs $@ $(OBJS)

radar: RadarKitTest/main.c
	$(CC) -o $@ $(CFLAGS) $< $(LDFLAGS)

clean:
	rm -f $(RKLIB)
	rm $(OBJS)

install:
	sudo cp -rp headers/RadarKit headers/RadarKit.h /usr/local/include/
	sudo cp -p libRadarKit.a /usr/local/lib/

uninstall:
	rm -rf /usr/local/include/RadarKit.h /usr/local/include/RadarKit
	rm -rf /usr/local/lib/libRadarKit.a

