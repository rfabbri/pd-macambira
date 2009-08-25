PD_DIR = ../../pd
PDP_DIR = ../pdp

GEM_OPENCV_VERSION = 0.1

# build flags

INCLUDES =  -I$(PD_DIR)/src -I.  -I$(PDP_DIR)/include -I$(PD_DIR)/src
CPPFLAGS  = -fPIC -DPD -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wno-unused -Wno-parentheses -Wno-switch \
    -DGEM_OPENCV_VERSION=\"$(GEM_OPENCV_VERSION)\" -g


UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
 INCLUDES += `pkg-config --cflags opencv`
 LDFLAGS =  --export-dynamic -shared
 LIBS = `pkg-config --libs opencv`
 EXTENSION = pd_linux
endif
ifeq ($(UNAME),Darwin)
 export MACOSX_DEPLOYMENT_TARGET = 10.3
 OPENCV_DIR = /Users/pd/opencv-1.0.0
 INCLUDES += -I$(OPENCV_DIR)/cv/include -I$(OPENCV_DIR)/cxcore/include -I$(OPENCV_DIR)/cvaux/include -I/sw/include
 LDFLAGS =  -bundle -undefined dynamic_lookup
 LIBS =  -lm $(OPENCV_DIR)/build/cv/src/.libs/lib_cv.a -L/sw/lib
 EXTENSION = pd_darwin
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
 OPENCV_DIR = opencv-1.0.0
 INCLUDES += -I$(OPENCV_DIR)/cv/include -I$(OPENCV_DIR)/cxcore/include -I$(OPENCV_DIR)/cvaux/include
 CPPFLAGS += -mms-bitfields -DMSW -DNT $(WINDOWS_HACKS)
 LDFLAGS += -shared
 LIBS = -L$(pd_src)/bin -L$(pd_src)/obj -lpd -lkernel32 -luser32 -lgdi32 \
	-lm $(OPENCV_DIR)/build/cv/src/.libs/lib_cv.a
endif

.SUFFIXES = $(EXTENSION)

SOURCES = pdp_opencv_threshold.c pdp_opencv_edge.c pdp_opencv_distrans.c pdp_opencv_laplace.c pdp_opencv_motempl.c pdp_opencv_morphology.c pdp_opencv_haarcascade.c pdp_opencv_contours_convexity.c pdp_opencv_contours_boundingrect.c pdp_opencv_bgsubstract.c pdp_opencv_lk.c pdp_opencv_floodfill.c pdp_opencv_hist_compare.c pdp_opencv_dft.c pdp_opencv_knear.cc pdp_opencv_hu_moments.c pdp_opencv_hu_compare.cc pdp_opencv_pgh_compare.cc pdp_opencv_bgstats.cc pdp_opencv_surf.cc pdp_opencv_athreshold.c pdp_opencv_hough_lines.cc pdp_opencv_channels.cc pdp_opencv_hough_circles.cc pdp_opencv_camshift.cc

all: $(SOURCES:.c=.$(EXTENSION)) $(SOURCES:.cc=.$(EXTENSION))

%.$(EXTENSION): %.o
	gcc $(LDFLAGS) -o $*.$(EXTENSION) $*.o $(LIBS)

.cc.o:
	g++ $(CPPFLAGS) $(INCLUDES) -o $*.o -c $*.cc

.c.o:
	gcc $(CPPFLAGS) $(INCLUDES) -o $*.o -c $*.c

install:
	cp -f --remove-destination *.pd $(PD_DIR)/doc/5.reference

clean:
	rm -f pdp_opencv*.o
	rm -f pdp_opencv*.$(EXTENSION)

distro: clean all
	rm *.o

