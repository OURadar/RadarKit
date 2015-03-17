CFLAGS = -std=gnu99 -O2 -Wall -Wno-unknown-pragmas -fPIC -D_GNU_SOURCE -msse -msse2 -msse3 -I ./ -I /usr/local/include
LDFLAGS = -L ./ -L /usr/local/lib
OBJS = RKRadar.o
RKLIB = libRadarKit.a

all: $(RKLIB)

$(OBJS): %.o: RadarKit/%.c RadarKit/%.h
	gcc $(CFLAGS) -c $< -o $@

.c:
	gcc $(CFLAGS) -c $< -o $@

$(RKLIB): $(OBJS)
	ar rvcs $@ $(OBJS)

