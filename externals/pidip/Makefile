PD_DIR = /usr/local/pd
PDP_DIR = /usr/local/pd/pdp
FFMPEG_SOURCE_DIR = /SOURCES/ffmpeg

PDP_PIDIP_LIBS =  -lbz2 -lz -ldl -lmp3lame -logg -lvorbis -lvorbisenc /SOURCES/ffmpeg/libavformat/libavformat.a /SOURCES/ffmpeg/libavcodec/libavcodec.a
IMLIB_CFLAGS = -I/usr/local/include -I/usr/X11R6/include
IMLIB_LIBS = -L/usr/local/lib -lImlib2 -lttf -lm -ldl -lXext -lXext -lX11 -L/usr/X11R6/lib
MAGICK_CFLAGS = -I/usr/X11R6/include -g -O2 -Wall 
MAGICK_LIBS = -L/usr/X11R6/lib -lMagick -lMagick -ltiff -lfreetype -ljasper -ljpeg -lpng -lXext -lSM -lICE -lX11 -lbz2 -lxml2 -lz -lpthread -lm -L/usr/local/lib -L/usr/X11R6/lib -L/usr/X11R6/lib -L/usr/local/lib
PDP_PIDIP_INCLUDES =  -I/usr/local/pd/src -I. -I/usr/local/pd/pdp/include -I../include  -I/SOURCES/ffmpeg/libavcodec -I/SOURCES/ffmpeg/libavformat
PDP_PIDIP_VERSION = 0.12.14
MPEG4IP_CFLAGS = 

PDP_PIDIP_DISTRO = /mnt/c/ydegoyon.free.fr/pidip-$(PDP_PIDIP_VERSION)
PDP_PIDIP_TARBALL = $(PDP_PIDIP_DISTRO).tar.gz


PDP_PIDIP_CFLAGS  = $(MPEG4IP_CFLAGS) $(MAGICK_CFLAGS) $(IMLIB_CFLAGS) \
    -DPD -DX_DISPLAY_MISSING -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch \
    -DPDP_PIDIP_VERSION=\"$(PDP_PIDIP_VERSION)\" -g

PDP_PIDIP_CPPFLAGS  = $(MPEG4IP_CFLAGS) $(MAGICK_CFLAGS) $(IMLIB_CFLAGS) \
    -DPD -DX_DISPLAY_MISSING -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch \
    -DPDP_PIDIP_VERSION=\"$(PDP_PIDIP_VERSION)\" -g

all: pidip.pd_linux

pdp_pidip_all:
	make -C system
	make -C modules

pidip.pd_linux: pdp_pidip_all
	rm -f pidip.pd_linux
	g++ -export_dynamic -shared -o pidip.pd_linux modules/*.o system/*.o $(PDP_PIDIP_LIBS) $(IMLIB_LIBS) $(MAGICK_LIBS)
	strip --strip-unneeded pidip.pd_linux

clean:
	rm -f */*.o
	rm -f pidip.pd_linux

install:
	cp fonts/* /usr/X11R6/lib/X11/fonts/TTF
	cp -f --remove-destination doc/* $(PD_DIR)/doc/5.reference

distro: clean all
	rm -f */*.o
	rm -rf autom4te.cache
	cd .. && cp -rf pidip /tmp/pidip-$(PDP_PIDIP_VERSION)
	cd /tmp && tar vczf $(PDP_PIDIP_TARBALL) pidip-$(PDP_PIDIP_VERSION)
	cp /mnt/c/ydegoyon.free.fr/pidip-$(PDP_PIDIP_VERSION).tar.gz /mnt/c/Yves/Software
	rm -rf /tmp/pidip-$(PDP_PIDIP_VERSION)

.c.o:
	gcc $(PDP_PIDIP_CFLAGS) $(PDP_PIDIP_INCLUDES) -o $*.o -c $*.c

.cpp.o:
	g++ $(PDP_PIDIP_CPPFLAGS) $(PDP_PIDIP_INCLUDES) -o $*.o -c $*.cpp
