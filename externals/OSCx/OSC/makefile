###############################
# change these
IFLAGS = -I. -I../../../src/pd-0.35-0/src
INSTALL_PREFIX=/usr

# should be oke below ..
EXT = o
#DEFS =  -DHAVE_LIBC=1 -DHAVE_LIBM=1 -DHAVE_LIBPTHREAD=1 -DSTDC_HEADERS=1 -DHAVE_FCNTL_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_UNISTD_H=1 -DHAVE_GETPAGESIZE=1 -DHAVE_MMAP=1 -DHAVE_SELECT=1 -DHAVE_SOCKET=1 -DHAVE_STRERROR=1  -DPD_VERSION_MINOR=32
DEFS =
CC = gcc
CXX = c++
LD = ld
AFLAGS = 
LFLAGS = -export_dynamic  -shared
WFLAGS =

# DEBUG = -DDEBUG -g
DEBUG = 

VERSION = \"$(shell cat VERSION)\"

.SUFFIXES: .$(EXT)

PDCFLAGS = -O2 $(DEFS) $(IFLAGS) $(WFLAGS) $(LFLAGS) $(AFLAGS) -DVERSION=$(VERSION)
CFLAGS = -O2 $(DEFS) $(IFLAGS) $(WFLAGS) -DVERSION=$(VERSION) $(DEBUG)
CXXFLAGS = $(CFLAGS)
LIBOSC = 	../libOSC/libOSC.a
LIBS = -lpthread -lm -lc 
SOURCES = htmsocket.c OSC-pattern-match.c sendOSC.c dumpOSC.c routeOSC.c
TARGETS = $(SOURCES:.c=.$(EXT)) 

all: $(TARGETS) 

OSC: $(TARGETS)
	cc -c $(CFLAGS) -DPD OSC.c
	$(LD) -export_dynamic  -shared -o OSC.pd_linux *.o $(LIBS) $(LIBOSC)
	strip --strip-unneeded OSC.pd_linux

clean::
	-rm *.$(EXT) *.pd_linux *~

distclean: clean
	-rm config.cache config.log config.status makefile


distcleancvs:
	-rm -r CVS reference/CVS


.c.o:
	$(CC) -c -o $@ $(CFLAGS) -DPD $*.c

# cp $@ $*_stat.o

#.o.pd_linux:
#	$(CC) -o $@ $(PDCFLAGS) -DPD $*.o



install::
	install -d $(INSTALL_PREFIX)/lib/pd/externs
#	install -m 644 *.pd_linux $(INSTALL_PREFIX)/lib/pd/externs
	install -m 644 OSC.pd_linux $(INSTALL_PREFIX)/lib/pd/externs
	install -d $(INSTALL_PREFIX)/lib/pd/doc/5.reference/OSC
	install -m 644 ../doc/*.pd $(INSTALL_PREFIX)/lib/pd/doc/5.reference/OSC


#dist: distclean
#	(cd ..;tar czvf ggext.tgz ggext)
