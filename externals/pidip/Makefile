PD_DIR = /usr/local/pd
PDP_DIR = /usr/local/pd/pdp
FFMPEG_SOURCE_DIR = /SOURCES/ffmpeg

LIBS = -lvorbisenc -lvorbis -logg -lmp3lame -ldl -lz -lbz2 -lMagick  -lbz2 -lz -ldl -lmp3lame -logg -lvorbis -lvorbisenc
IMLIB_CFLAGS = -I/usr/local/include -I/usr/X11R6/include
IMLIB_LIBS = -L/usr/local/lib -lImlib2 -lttf -lm -ldl -lXext -lXext -lX11 -L/usr/X11R6/lib
AALIB_CFLAGS = -I/usr/local/include
AALIB_LIBS = -L/usr/local/lib -Wl,-rpath,/usr/local/lib -laa -lm -L/usr/X11R6/lib -lX11 -lgpm -lslang
MAGICK_CFLAGS = -I/usr/X11R6/include -g -O2 -Wall 
MAGICK_LIBS = -L/usr/X11R6/lib -lMagick -ltiff -lfreetype -ljasper -ljpeg -lpng -lXext -lSM -lICE -lX11 -lbz2 -lxml2 -lz -lpthread -lm -L/usr/local/lib -L/usr/X11R6/lib -L/usr/X11R6/lib -L/usr/local/lib
PDP_PIDIP_VERSION = 0.12.4

PDP_PIDIP_DISTRO = /mnt/c/ydegoyon.free.fr/pidip-$(PDP_PIDIP_VERSION)
PDP_PIDIP_TARBALL = $(PDP_PIDIP_DISTRO).tar.gz
# build flags

PDP_PIDIP_INCLUDE =  -I$(PD_DIR)/src -I. -I$(PDP_DIR)/include  -I$(FFMPEG_SOURCE_DIR)/libavcodec -I$(FFMPEG_SOURCE_DIR)/libavformat -I../include
PDP_PIDIP_CFLAGS  = $(MAGICK_CFLAGS) $(AALIB_CFLAGS) $(IMLIB_CFLAGS) \
    -DPD -DX_DISPLAY_MISSING -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch \
    -DPDP_PIDIP_VERSION=\"$(PDP_PIDIP_VERSION)\" \
   -g
PDP_PIDIP_CPPFLAGS  = -O

all: pidip.pd_linux

pdp_pidip_all:
	make -C system
	make -C modules

pidip.pd_linux: pdp_pidip_all
	rm -f pidip.pd_linux
	gcc -export_dynamic -shared -o pidip.pd_linux modules/*.o system/*.o $(FFMPEG_SOURCE_DIR)/libavformat/libavformat.a $(FFMPEG_SOURCE_DIR)/libavcodec/libavcodec.a $(LIBS) $(IMLIB_LIBS) $(AALIB_LIBS) $(MAGICK_LIBS)

clean:
	rm -f */*.o
	rm -f pidip.pd_linux
	rm -f *~

install:
	cp fonts/* /usr/X11R6/lib/X11/fonts/TTF

distro: clean all
	rm */*.o
	strip --strip-unneeded pidip.pd_linux
	cd .. && cp -rf pidip /tmp/pidip-$(PDP_PIDIP_VERSION)
	cd /tmp && tar vczf $(PDP_PIDIP_TARBALL) pidip-$(PDP_PIDIP_VERSION)
	cp /mnt/c/ydegoyon.free.fr/pidip-$(PDP_PIDIP_VERSION).tar.gz /mnt/c/Yves
	rm -rf /tmp/pidip-$(PDP_PIDIP_VERSION)

.c.o:
	gcc $(PDP_PIDIP_CFLAGS) $(PDP_PIDIP_INCLUDE) -o $*.o -c $*.c

.cpp.o:
	g++ $(PDP_PIDIP_CPPFLAGS) $(PDP_PIDIP_INCLUDE) -o $*.o -c $*.cpp

