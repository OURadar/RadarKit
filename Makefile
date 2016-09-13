CFLAGS = -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -fPIC -D_GNU_SOURCE -msse -msse2 -msse3 -I ./ -I headers/ -I /usr/local/include
LDFLAGS = -L ./ -L /usr/local/lib
OBJS = RKRadar.o
RKLIB = libRadarKit.a

all: $(RKLIB)

$(OBJS): %.o: source/%.c headers/RadarKit/%.h
	gcc $(CFLAGS) -c $< -o $@

.c:
	gcc $(CFLAGS) -c $< -o $@

$(RKLIB): $(OBJS)
	ar rvcs $@ $(OBJS)

install:
	sudo cp -rp headers/RadarKit /usr/local/include/
	sudo cp -p libRadarKit.a /usr/local/lib/
