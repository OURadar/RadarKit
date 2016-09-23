UNAME := $(shell uname)

CFLAGS = -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -fPIC -msse -msse2 -msse3 -mavx -I /usr/local/include -I /usr/include
LDFLAGS = -L /usr/local/lib -L /usr/lib64 -lRadarKit -lpthread -lfftw3f
OBJS = RadarKit.o RKRadar.o RKFoundation.o RKMisc.o RKPulseCompression.o RKServer.o RKLocalCommandCenter.o
RKLIB = libRadarKit.a

ifeq ($(UNAME), Darwin)
# Mac OS X
CC = clang
CFLAGS += -D_DARWIN_C_SOURCE -Wno-deprecated-declarations
else
CFLAGS += -D_GNU_SOURCE
endif

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

