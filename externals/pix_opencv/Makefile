PD_DIR = /usr/src/pd-svn/trunk/pd
GEM_DIR = /usr/src/pd-svn/trunk/Gem

GEM_OPENCV_VERSION = 0.2

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

.SUFFIXES = $(EXTENSION)

SOURCES = pix_opencv_edge.cc pix_opencv_laplace.cc pix_opencv_morphology.cc pix_opencv_distrans.cc pix_opencv_motempl.cc pix_opencv_haarcascade.cc pix_opencv_contours_boundingrect.cc pix_opencv_bgsubstract.cc pix_opencv_contours_convexity.cc pix_opencv_dft.cc pix_opencv_lk.cc pix_opencv_hist_compare.cc pix_opencv_knear.cc pix_opencv_threshold.cc pix_opencv_floodfill.cc pix_opencv_athreshold.cc pix_opencv_bgstats.cc pix_opencv_camshift.cc pix_opencv_hu_compare.cc pix_opencv_pgh_compare.cc pix_opencv_hough_circles.cc pix_opencv_hough_lines.cc pix_opencv_hu_moments.cc pix_opencv_contours_convexhull.cc pix_opencv_colorfilt.cc
SOURCES_OPT = 

all: $(SOURCES:.cc=.$(EXTENSION)) $(SOURCES_OPT:.cc=.$(EXTENSION))

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
