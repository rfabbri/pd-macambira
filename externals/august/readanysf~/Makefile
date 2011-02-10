# Edit these two variables to suit your system.
# You need both gavl and gmerlin_avdec libs to compile


GAVLPREFIX=/usr/local
#GAVLPREFIX=/opt/gmerlin
PD_INCLUDES=/usr/include


# SHOULDN'T REALLY NEED TO EDIT BELOW HERE

VERSION=0.43

UNAME := $(shell uname)
ifneq ($(UNAME), Darwin)
# simplistic approach to handle Debians non-linux architectures (kFreeBSD,
# kHurd) the same as linux
UNAME=Linux
endif

ifeq ($(UNAME), Linux)
TARGET=pd_linux
STRIP=strip --strip-unneeded
PD_LDFLAGS =  -L$(GAVLPREFIX)/lib -lpthread  -lgavl -lgmerlin_avdec
GAVLPATH=$(GAVLPREFIX)/include
CXXFLAGS=-Wall
PD_CXXFLAGS =  -I./  -I$(GAVLPATH) -I$(GAVLPATH)/gavl -I$(GAVLPATH)/gmerlin -I$(PD_INCLUDES)
else
# assume darwin here
GAVLPATH=/sw/include
PD_INCLUDES=/Applications/Pd-extended.app/Contents/Resources/include/
TARGET=pd_darwin
STRIP=strip -x
PD_CXXFLAGS =  -I./  -I$(GAVLPATH) -I$(GAVLPATH)/gavl -I$(GAVLPATH)/gmerlin -I$(PD_INCLUDES)
PD_CXXFLAGS += -I/sw/include -fast
PD_LDFLAGS = -bundle -undefined dynamic_lookup -L/sw/lib -lgavl -lgmerlin_avdec
#PD_LDFLAGS += -bundle -bundle_loader $(pd_src)/bin/pd -undefined dynamic_lookup \
#		-L/sw/lib -weak_framework Carbon -lc -L/sw/lib -lgavl -lgmerlin_avdec
# os 10.4
#PD_CXXFLAGS += -mmacosx-version-min=10.4  -arch i386  -isysroot /Developer/SDKs/MacOSX10.4u.sdk 
#PD_LDFLAGS =  -L/sw/lib -lgavl -lgmerlin_avdec \
#        -dynamiclib -undefined dynamic_lookup  -lsupc++ -mmacosx-version-min=10.4 \
#        -lSystem.B -arch i386  -isysroot /Developer/SDKs/MacOSX10.4u.sdk
endif

##############################################
PD_CXXFLAGS +=  -fPIC 

PD_CXXFLAGS += $(CXXFLAGS)
PD_LDFLAGS  += $(LDFLAGS)

all: $(TARGET) 

pd_linux: src/readanysf~.cpp  objs/FifoVideoFrames.o objs/FifoAudioFrames.o objs/ReadMedia.o
	$(CXX)  -shared  -o  readanysf~.pd_linux  $(PD_CXXFLAGS) $(PD_LDFLAGS) \
	src/readanysf~.cpp \
	objs/FifoAudioFrames.o \
	objs/FifoVideoFrames.o \
	objs/ReadMedia.o 
	$(STRIP) readanysf~.pd_linux

pd_darwin: src/readanysf~.cpp  objs/FifoVideoFrames.o objs/FifoAudioFrames.o objs/ReadMedia.o
	$(CXX)  $(PD_LDFLAGS)  -o  readanysf~.pd_darwin  $(PD_CXXFLAGS)  \
	src/readanysf~.cpp \
	objs/FifoAudioFrames.o \
	objs/FifoVideoFrames.o \
	objs/ReadMedia.o 
	$(STRIP) readanysf~.pd_darwin
	mkdir -p readanysf~$(VERSION)_MacOSX-Intel
	mkdir -p readanysf~$(VERSION)_MacOSX-Intel/readanysf~
	cp readanysf~.pd_darwin readanysf~-help.pd readanysf~$(VERSION)_MacOSX-Intel/readanysf~
	cp READMEmacpkg.txt anysndfiler.pd readanysf~$(VERSION)_MacOSX-Intel/
	mv readanysf~$(VERSION)_MacOSX-Intel/READMEmacpkg.txt readanysf~$(VERSION)_MacOSX-Intel/README.txt
	./embed-MacOSX-dependencies.sh readanysf~$(VERSION)_MacOSX-Intel/readanysf~
	tar -cvf readanysf~$(VERSION)_MacOSX-Intel.tar readanysf~$(VERSION)_MacOSX-Intel/
	gzip readanysf~$(VERSION)_MacOSX-Intel.tar

objs/ReadMedia.o: src/ReadMedia.cpp src/ReadMedia.h objs/FifoAudioFrames.o objs/FifoVideoFrames.o objs/
	$(CXX)  -c -o objs/ReadMedia.o src/ReadMedia.cpp $(PD_CXXFLAGS)

objs/FifoAudioFrames.o: src/FifoAudioFrames.cpp src/FifoAudioFrames.h objs/
	$(CXX)  -c -o objs/FifoAudioFrames.o src/FifoAudioFrames.cpp $(PD_CXXFLAGS)

objs/FifoVideoFrames.o: src/FifoVideoFrames.cpp src/FifoVideoFrames.h objs/
	$(CXX)  -c -o objs/FifoVideoFrames.o src/FifoVideoFrames.cpp $(PD_CXXFLAGS)

clean:
	if [ -d readanysf~$(VERSION)_MacOSX-Intel ]; then rm -rf readanysf~$(VERSION)_MacOSX-Intel; fi; 
	if [ -f readanysf~$(VERSION)_MacOSX-Intel.tar.gz ]; then rm -rf readanysf~$(VERSION)_MacOSX-Intel.tar.gz; fi; 
	rm -f objs/*.o readanysf~.pd_* 
	rm -rf objs

objs/:
	mkdir $@

