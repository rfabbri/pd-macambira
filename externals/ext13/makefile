
EXT = pd_linux
DEFS =  -DHAVE_LIBC=1 -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1 -DSTDC_HEADERS=1 -DHAVE_FCNTL_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_UNISTD_H=1 -DHAVE_GETPAGESIZE=1 -DHAVE_MMAP=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRERROR=1  -DPD_VERSION_MINOR=32
CC = gcc
CXX = c++
LD = ld
AFLAGS = 
LFLAGS = -export_dynamic  -shared
WFLAGS =
IFLAGS = -I./include
PREFIX=/usr/local
INSTALL_PREFIX=$(PREFIX)

VERSION = \"$(shell cat VERSION)\"
TARNAME =  ext13.$(shell cat VERSION).tar.gz

.SUFFIXES: .$(EXT)

PDCFLAGS = -g -O2 $(DEFS) $(IFLAGS) $(WFLAGS) $(LFLAGS) $(AFLAGS) -DVERSION=$(VERSION)
CFLAGS = -g -O2 $(DEFS) $(IFLAGS) $(WFLAGS) -DVERSION=$(VERSION)
CXXFLAGS = $(CFLAGS)

#LIBS = -lc -lm 
LIBS = -lpthread -lm -lc 
SOURCES = ftos.c kalashnikov.c openpatch.c sfread.c streamin13~.c strippath.c send13~.c receive13~.c catch13~.c throw13~.c filesize.c messages.c pipewrite~.c piperead~.c streamout13~.c wavinfo.c mandelbrot.c mandelbrot~.c scramble~.c promiscous~.c ossmixer.c send13.c receive13.c cdplayer.c
TARGETS = $(SOURCES:.c=.$(EXT)) 

all: $(TARGETS) 

ext13: $(TARGETS)
	cc -c $(CFLAGS) -DPD ext13.c
	$(LD) -export_dynamic  -shared -o ext13.pd_linux *.o $(LIBS)
	strip --strip-unneeded ext13.pd_linux

clean::
	-rm *.$(EXT) *.o 

distclean: clean
	-rm config.cache config.log config.status makefile



.c.o:
	$(CC) -c -o $@ $(CFLAGS) -DPD $*.c

# cp $@ $*_stat.o

.o.pd_linux:
	$(CC) -o $@ $(PDCFLAGS) -DPD $*.o



install::
	install -d $(INSTALL_PREFIX)/pd/externs
	install -m 644 *.$(EXT) $(INSTALL_PREFIX)/pd/externs
	-install -m 644 ext13.pd_linux $(INSTALL_PREFIX)/pd/externs
	install -m 644 doc/*.pd $(INSTALL_PREFIX)/pd/doc/5.reference


dist: distclean
	(cd ..;tar czvf $(TARNAME) ext13)
