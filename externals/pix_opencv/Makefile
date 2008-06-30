PD_DIR = ../../pd
GEM_DIR = ../../Gem
OPENCV_DIR = opencv-1.0.0

LIBS =  -lm $(OPENCV_DIR)/build/cv/src/.libs/lib_cv.a
GEM_OPENCV_VERSION = 0.1

# build flags

GEM_OPENCV_INCLUDE =  -I$(PD_DIR)/src -I.  -I$(GEM_DIR)/src -I$(PD_DIR)/src \
	-I$(OPENCV_DIR)/cv/include	-I$(OPENCV_DIR)/cxcore/include
GEM_OPENCV_CPPFLAGS  = -DPD -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wno-unused -Wno-parentheses -Wno-switch \
    -DGEM_OPENCV_VERSION=\"$(GEM_OPENCV_VERSION)\" -g


UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
 LDFLAGS =  -export_dynamic -shared
 EXTENSION = pd_linux
endif
ifeq ($(UNAME),Darwin)
 LDFLAGS =  -bundle -undefined dynamic_lookup
 EXTENSION = pd_darwin
endif

all: pix_opencv_edge.$(EXTENSION) pix_opencv_laplace.$(EXTENSION) pix_opencv_morphology.$(EXTENSION) pix_opencv_distrans.$(EXTENSION) pix_opencv_motempl.$(EXTENSION) pix_opencv_haarcascade.$(EXTENSION) pix_opencv_contours_boundingrect.$(EXTENSION) pix_opencv_bgsubstract.$(EXTENSION) pix_opencv_contours_convexity.$(EXTENSION)

pix_opencv_contours_convexity.$(EXTENSION): pix_opencv_contours_convexity.o
	rm -f pix_opencv_contours_convexity.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_contours_convexity.$(EXTENSION) pix_opencv_contours_convexity.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_contours_boundingrect.$(EXTENSION): pix_opencv_contours_boundingrect.o
	rm -f pix_opencv_contours_boundingrect.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_contours_boundingrect.$(EXTENSION) pix_opencv_contours_boundingrect.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_haarcascade.$(EXTENSION): pix_opencv_haarcascade.o
	rm -f pix_opencv_haarcascade.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_haarcascade.$(EXTENSION) pix_opencv_haarcascade.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_motempl.$(EXTENSION): pix_opencv_motempl.o
	rm -f pix_opencv_motempl.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_motempl.$(EXTENSION) pix_opencv_motempl.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_distrans.$(EXTENSION): pix_opencv_distrans.o
	rm -f pix_opencv_distrans.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_distrans.$(EXTENSION) pix_opencv_distrans.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_morphology.$(EXTENSION): pix_opencv_morphology.o
	rm -f pix_opencv_morphology.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_morphology.$(EXTENSION) pix_opencv_morphology.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_laplace.$(EXTENSION): pix_opencv_laplace.o
	rm -f pix_opencv_laplace.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_laplace.$(EXTENSION) pix_opencv_laplace.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_edge.$(EXTENSION): pix_opencv_edge.o 
	rm -f pix_opencv_edge.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_edge.$(EXTENSION) pix_opencv_edge.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_bgsubstract.$(EXTENSION): pix_opencv_bgsubstract.o 
	rm -f pix_opencv_bgsubstract.$(EXTENSION)
	gcc $(LDFLAGS) -o pix_opencv_bgsubstract.$(EXTENSION) pix_opencv_bgsubstract.o $(LIBS) `pkg-config --libs opencv`

clean:
	rm -f pix_opencv*.o
	rm -f pix_opencv*.$(EXTENSION)

distro: clean all
	rm *.o

.cc.o:
	g++ $(GEM_OPENCV_CPPFLAGS) $(GEM_OPENCV_INCLUDE) `pkg-config --cflags opencv` -o $*.o -c $*.cc

.c.o:
	gcc $(GEM_OPENCV_CPPFLAGS) $(GEM_OPENCV_INCLUDE) -o $*.o -c $*.c

install:
	cp -f --remove-destination *.pd $(PD_DIR)/doc/5.reference
