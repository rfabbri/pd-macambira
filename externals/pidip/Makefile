PD_DIR = /usr/local/pd
PDP_DIR = /usr/local/pd/pdp
FFMPEG_SOURCE_DIR = /SOURCES/ffmpeg

PDP_PIDIP_LIBS =  -lbz2 -lz -ldl -lmp3lame -logg -lvorbis -lvorbisenc
IMLIB_CFLAGS = -I/usr/local/include -I/usr/X11R6/include
IMLIB_LIBS = -L/usr/local/lib -lImlib2 -lfreetype -lz -lm -ldl -lXext -lXext -lX11 -L/usr/X11R6/lib
MAGICK_CFLAGS = -I/usr/X11R6/include -g -O2 -Wall -pthread 
MAGICK_LIBS = -L/usr/X11R6/lib -lMagick -lMagick -ltiff -lfreetype -ljpeg -lpng -lXext -lXt -lSM -lICE -lX11 -lbz2 -lxml2 -lz -lpthread -lm -lpthread -L/usr/local/lib -L/usr/X11R6/lib -lfreetype -lz -L/usr/lib

# what are those? seem to be related to xorg
#MAGICK_LIBS += -ldpstk -ldps

#THEORA_LIBS =  /usr/lib/libtheora.a /usr/lib/libogg.a /usr/lib/libvorbis.a /usr/lib/libvorbisenc.a
#THEORA_LIBS += /usr/local/lib/libtheora.a /usr/local/lib/libogg.a /usr/local/lib/libvorbis.a /usr/local/lib/libvorbisenc.a
THEORA_LIBS =  -ltheora -logg -lvorbis -lvorbisenc
PDP_PIDIP_INCLUDES =  -I/usr/local/pd/src -I. -I/usr/local/pd/pdp/include -I../include -I../charmaps
PDP_PIDIP_VERSION = 0.12.20
MPEG4IP_CFLAGS = 

PDP_PIDIP_DISTRO = /mnt/c/ydegoyon.free.fr/pidip-$(PDP_PIDIP_VERSION)
PDP_PIDIP_TARBALL = $(PDP_PIDIP_DISTRO).tar.gz


PDP_PIDIP_CFLAGS  = $(MPEG4IP_CFLAGS) $(MAGICK_CFLAGS) $(IMLIB_CFLAGS) \
    -DPD -DX_DISPLAY_MISSING -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wstrict-prototypes -fPIC \
    -Wno-unused -Wno-parentheses -Wno-switch \
    -DPDP_PIDIP_VERSION=\"$(PDP_PIDIP_VERSION)\" -g

PDP_PIDIP_CPPFLAGS  = $(MPEG4IP_CFLAGS) $(MAGICK_CFLAGS) $(IMLIB_CFLAGS) \
    -DPD -DX_DISPLAY_MISSING -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wstrict-prototypes -fPIC \
    -Wno-unused -Wno-parentheses -Wno-switch \
    -DPDP_PIDIP_VERSION=\"$(PDP_PIDIP_VERSION)\" -g

all: pidip.pd_linux

pdp_pidip_all:
	make -C system
	make -C modules

pidip.pd_linux: pdp_pidip_all
	rm -f pidip.pd_linux
	g++ -export_dynamic -shared -o pidip.pd_linux modules/*.o system/*.o $(THEORA_LIBS) $(PDP_PIDIP_LIBS) $(IMLIB_LIBS) $(MAGICK_LIBS)
	strip --strip-unneeded pidip.pd_linux

clean:
	rm -f */*.o
	rm -f pidip.pd_linux

install:
	if test ! -d /usr/X11R6/lib/X11/fonts/TTF; then mkdir /usr/X11R6/lib/X11/fonts/TTF; fi
	cp fonts/* /usr/X11R6/lib/X11/fonts/TTF
	cp -f --remove-destination doc/* $(PD_DIR)/doc/5.reference

distro: clean all
	rm -f */*.o
	rm -rf autom4te.cache
	cd .. && cp -rf pidip /tmp/pidip-$(PDP_PIDIP_VERSION)
	cd /tmp && tar vczf $(PDP_PIDIP_TARBALL) pidip-$(PDP_PIDIP_VERSION)
	cp /mnt/c/ydegoyon.free.fr/pidip-$(PDP_PIDIP_VERSION).tar.gz /mnt/c/Yves/Software
	rm -rf /tmp/pidip-$(PDP_PIDIP_VERSION)

pdp_colorgrid.o: pdp_colorgrid.c
	./tk2c.bash < $*.tk > $*.tk2c
	PWD=`pwd`; cc -DPWD=\"$(PWD)\" $(PDP_PIDIP_CFLAGS) $(PDP_PIDIP_INCLUDES) -o $*.o -c $*.c

.c.o:
	gcc $(PDP_PIDIP_CFLAGS) $(PDP_PIDIP_INCLUDES) -o $*.o -c $*.c

.cpp.o:
	g++ $(PDP_PIDIP_CPPFLAGS) $(PDP_PIDIP_INCLUDES) -o $*.o -c $*.cpp
