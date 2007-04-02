
EXT = pd_linux 
DEFS = -DUNIX -DLINUX -DHAVE_LIBC=1 -DHAVE_LIBM=1 -DSTDC_HEADERS=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_UNISTD_H=1 -DHAVE_GETPAGESIZE=1 -DHAVE_MMAP=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRERROR=1  -DPD_VERSION_MINOR=36
CC = gcc
CXX = c++
LD = ld
AFLAGS = 
LFLAGS = -export_dynamic  -shared
WFLAGS =
IFLAGS = -I. -I../../pd/src
PREFIX=/usr/
INSTALL_PREFIX=$(PREFIX)

VERSION = \"$(shell cat VERSION)\"
TARNAME =  cxc-$(shell cat VERSION).tar.gz

.SUFFIXES: .$(EXT)

PDCFLAGS = -g -O2 $(DEFS) $(IFLAGS) $(WFLAGS) $(LFLAGS) $(AFLAGS) -DVERSION=$(VERSION)
CFLAGS = -g -O2 $(DEFS) $(IFLAGS) $(WFLAGS) -DVERSION=$(VERSION)
CXXFLAGS = $(CFLAGS)

LIBS = -lc -lm 
#LIBS = -lpthread -lm -lc 
SOURCES = ENV.c ascseq.c ascwave.c bangfilt.c bangfilt2.c binshift.c cx.c ixprint.c mean.c utime.c proc.c randomix.c reson~.c sigdelta.c split.c
TARGETS = $(SOURCES:.c=.$(EXT)) 

# all: $(TARGETS) 
all: cxc

cxc: $(TARGETS)
#	cc -c $(CFLAGS) -o cxc.o -DPD cx.c
	$(LD) $(LFLAGS) -o cxc.$(EXT) *.o $(LIBS)
	strip --strip-unneeded cxc.$(EXT)

clean::
	-rm *.$(EXT) *.o 

distclean: clean
	-rm config.cache config.log config.status makefile



.c.o:
	$(CC) -c -o $@ $(CFLAGS) -DPD $*.c

# cp $@ $*_stat.o

.o.$(EXT):
	$(CC) -o $@ $(PDCFLAGS) -DPD $*.o



install::
	install -d $(INSTALL_PREFIX)/pd/externs
#	install -m 644 *.$(EXT) $(INSTALL_PREFIX)/pd/externs
	-install -m 644 cxc.$(EXT) $(INSTALL_PREFIX)/pd/externs
	install -m 644 reference/*.pd $(INSTALL_PREFIX)/pd/doc/5.reference


dist: distclean
	(cd ..;tar czvf $(TARNAME) cxc)
