PD_DIR = /usr/src/pd-0.40-2
GEM_DIR = /usr/src/Gem

LIBS =  -lm
GEM_OPENCV_VERSION = 0.1

# build flags

GEM_OPENCV_INCLUDE =  -I$(PD_DIR)/src -I.  -I$(GEM_DIR)/src -I$(PD_DIR)/src
GEM_OPENCV_CPPFLAGS  = -DPD -O2 -funroll-loops -fomit-frame-pointer  -ffast-math \
    -Wall -W -Wno-unused -Wno-parentheses -Wno-switch \
    -DGEM_OPENCV_VERSION=\"$(GEM_OPENCV_VERSION)\" -g



all: pix_opencv_edge.pd_linux pix_opencv_laplace.pd_linux pix_opencv_morphology.pd_linux pix_opencv_distrans.pd_linux pix_opencv_motempl.pd_linux pix_opencv_haarcascade.pd_linux pix_opencv_contours_boundingrect.pd_linux pix_opencv_bgsubstract.pd_linux pix_opencv_contours_convexity.pd_linux

pix_opencv_contours_convexity.pd_linux: pix_opencv_contours_convexity.o
	rm -f pix_opencv_contours_convexity.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_contours_convexity.pd_linux pix_opencv_contours_convexity.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_contours_boundingrect.pd_linux: pix_opencv_contours_boundingrect.o
	rm -f pix_opencv_contours_boundingrect.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_contours_boundingrect.pd_linux pix_opencv_contours_boundingrect.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_haarcascade.pd_linux: pix_opencv_haarcascade.o
	rm -f pix_opencv_haarcascade.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_haarcascade.pd_linux pix_opencv_haarcascade.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_motempl.pd_linux: pix_opencv_motempl.o
	rm -f pix_opencv_motempl.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_motempl.pd_linux pix_opencv_motempl.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_distrans.pd_linux: pix_opencv_distrans.o
	rm -f pix_opencv_distrans.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_distrans.pd_linux pix_opencv_distrans.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_morphology.pd_linux: pix_opencv_morphology.o
	rm -f pix_opencv_morphology.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_morphology.pd_linux pix_opencv_morphology.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_laplace.pd_linux: pix_opencv_laplace.o
	rm -f pix_opencv_laplace.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_laplace.pd_linux pix_opencv_laplace.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_edge.pd_linux: pix_opencv_edge.o 
	rm -f pix_opencv_edge.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_edge.pd_linux pix_opencv_edge.o $(LIBS) `pkg-config --libs opencv`

pix_opencv_bgsubstract.pd_linux: pix_opencv_bgsubstract.o 
	rm -f pix_opencv_bgsubstract.pd_linux
	gcc -export_dynamic -shared -o pix_opencv_bgsubstract.pd_linux pix_opencv_bgsubstract.o $(LIBS) `pkg-config --libs opencv`

clean:
	rm -f *.o
	rm -f pix_opencv*.pd_linux

distro: clean all
	rm *.o

.cpp.o:
	g++ $(GEM_OPENCV_CPPFLAGS) $(GEM_OPENCV_INCLUDE) `pkg-config --cflags opencv` -o $*.o -c $*.cpp

.c.o:
	gcc $(GEM_OPENCV_CPPFLAGS) $(GEM_OPENCV_INCLUDE) -o $*.o -c $*.c

install:
	cp -f --remove-destination *.pd $(PD_DIR)/doc/5.reference
