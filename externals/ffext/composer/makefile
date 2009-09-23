DEBUG?=0

all: pd_linux

.SUFFIXES: .pd_linux

pd_linux: composer.pd_linux

LINUXCFLAGS = -DPD -DUNIX -DPIC -fPIC \
   -funroll-loops -fomit-frame-pointer \
   -Wall -W -Wno-shadow -Wstrict-prototypes \
   -Wno-unused -Wno-parentheses -Wno-switch
LINUXINCLUDE = -I/usr/src/pd/0.41.4/src -I/usr/include
LINUXLDFLAGS = -export_dynamic -shared

ifeq ($(DEBUG),1)
	LINUXCFLAGS += -O0 -g -ggdb -DDEBUG
	STRIP=test -f
else
	LINUXCFLAGS += -O2
	STRIP=strip --strip-unneeded
endif

composer.pd_linux: track.c pattern.c \
	track_proxy.c \
	composer.c common.h arraylist.h
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o composer.o -c composer.c
	$(LD) $(LINUXLDFLAGS) -o composer.pd_linux composer.o -lc -lm
	$(STRIP) composer.pd_linux

clean:
	rm -f *.o *.pd_linux


