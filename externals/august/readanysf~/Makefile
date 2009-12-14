# Edit these two variables to suit your system.
# You need both gavl and gmerlin_avdec libs to compile
#
GAVLPATH=/usr/local/include
PDPATH=/usr/local/include

##############################################
LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
   # do 64 bit stuff here, like set some CFLAGS
CFLAGS =  -fPIC -I./  -I$(GAVLPATH) -I$(GAVLPATH)/gavl -I$(GAVLPATH)/gmerlin -I$(PDPATH) -Wall
else
   # do 32 bit stuff here
CFLAGS =  -I./  -I$(GAVLPATH)  -I$(GAVLPATH)/gavl -I$(GAVLPATH)/gmerlin -I$(PDPATH) -Wall
endif

LDFLAGS =  -L/usr/local/lib -lpthread  -lgavl -lgmerlin_avdec 

LINUXCFLAGS = -O1 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch


all: pd_linux 

pd_linux: readanysf~.cpp Readsf.cpp Readsf.h  objs/FifoAudioFrames.o objs/Readsf.o FifoAudioFrames.h FifoAudioFrames.cpp
	g++  -shared  -o  readanysf~.pd_linux  $(CFLAGS) $(LDFLAGS) \
	readanysf~.cpp \
	objs/FifoAudioFrames.o \
	objs/Readsf.o 
	strip --strip-unneeded readanysf~.pd_linux

objs/Readsf.o: Readsf.cpp Readsf.h FifoAudioFrames.h 
	g++  -c -o objs/Readsf.o Readsf.cpp $(CFLAGS)

objs/FifoAudioFrames.o: FifoAudioFrames.cpp FifoAudioFrames.h 
	g++  -c -o objs/FifoAudioFrames.o FifoAudioFrames.cpp $(CFLAGS)

clean:
	rm objs/*.o readanysf~.pd_linux  readanysf~.pd_darwin
