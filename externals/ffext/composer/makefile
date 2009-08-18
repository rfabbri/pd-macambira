all: pd_linux

.SUFFIXES: .pd_linux

pd_linux: composer.pd_linux

LINUXCFLAGS = -DPD -DUNIX -DDEBUG -DPIC -fPIC \
   -funroll-loops -fomit-frame-pointer \
   -Wall -W -Wno-shadow -Wstrict-prototypes \
   -Wno-unused -Wno-parentheses -Wno-switch \
#LINUXINCLUDE = -I/usr/include
LINUXINCLUDE = -I/usr/src/pd/0.41.4/src
LINUXLDFLAGS = -export_dynamic -shared

composer.pd_linux: song.c track.c pattern.c \
	song_proxy.c track_proxy.c \
	composer.c common.h arraylist.h
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o composer.o -c composer.c
	$(LD) $(LINUXLDFLAGS) -o composer.pd_linux composer.o -lc -lm
	strip --strip-unneeded composer.pd_linux

clean:
	rm -f *.o *.pd_linux


