PD_DIR = ../pd
GEM_DIR = ../gem

GEM_OPENCV_VERSION = 0.1

# build flags

INCLUDES =  -I$(PD_DIR)/src -I.  -I$(GEM_DIR)/src -I$(PD_DIR)/src
CPPFLAGS  = -fPIC -DPD -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wno-unused -Wno-parentheses -Wno-switch \
    -DGEM_OPENCV_VERSION=\"$(GEM_OPENCV_VERSION)\" -g


UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
 INCLUDES += `pkg-config --cflags opencv`
 LDFLAGS =  -export_dynamic -shared
 LIBS = `pkg-config --libs opencv`
 EXTENSION = pd_linux
endif
ifeq ($(UNAME),Darwin)
 INCLUDES += -I/System/Library/Frameworks/OpenCV.framework/Headers/
 LDFLAGS =  -bundle -undefined dynamic_lookup
 LIBS =  -lm -framework OpenCV
 EXTENSION = pd_darwin
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
 OPENCV_DIR = opencv-1.0.0
 INCLUDES += -I$(OPENCV_DIR)/cv/include -I$(OPENCV_DIR)/cxcore/include
 CPPFLAGS += -mms-bitfields -DMSW -DNT $(WINDOWS_HACKS)
 LDFLAGS += -shared
 LIBS = -L$(pd_src)/bin -L$(pd_src)/obj -lpd -lkernel32 -luser32 -lgdi32 \
	-lm $(OPENCV_DIR)/build/cv/src/.libs/lib_cv.a
endif

.SUFFIXES = $(EXTENSION)

SOURCES = pix_opencv_edge.cc pix_opencv_laplace.cc pix_opencv_morphology.cc pix_opencv_distrans.cc pix_opencv_motempl.cc pix_opencv_haarcascade.cc pix_opencv_contours_boundingrect.cc pix_opencv_bgsubstract.cc pix_opencv_contours_convexity.cc pix_opencv_dft.cc pix_opencv_lk.cc pix_opencv_histo.cc pix_opencv_knear.cc pix_opencv_threshold.cc

all: $(SOURCES:.cc=.$(EXTENSION))

%.$(EXTENSION): %.o
	gcc $(LDFLAGS) -o $*.$(EXTENSION) $*.o $(LIBS)

.cc.o:
	g++ $(CPPFLAGS) $(INCLUDES) -o $*.o -c $*.cc

.c.o:
	gcc $(CPPFLAGS) $(INCLUDES) -o $*.o -c $*.c

install:
	cp -f --remove-destination *.pd $(PD_DIR)/doc/5.reference

clean:
	rm -f pix_opencv*.o
	rm -f pix_opencv*.$(EXTENSION)

distro: clean all
	rm *.o
