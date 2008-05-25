current: all

include Makefile.config

PDP_MOD = pdp_opencv_threshold.o pdp_opencv_edge.o pdp_opencv_distrans.o pdp_opencv_laplace.o pdp_opencv_motempl.o pdp_opencv_morphology.o pdp_opencv_haarcascade.o pdp_opencv_contours_convexity.o pdp_opencv_contours_boundingrect.o pdp_opencv_bgsubstract.o pdp_opencv_lk.o pdp_opencv_floodfill.o pdp_opencv.o 


# build basic image processing modules (derived from base class)
all: $(PDP_MOD)
	$(CC) -export_dynamic -shared -o pdp_opencv.pd_linux *.o $(PDP_LIBS) `pkg-config --cflags --libs opencv`

clean:
	rm -f *~
	rm -f *.o
	rm -f *.pd_linux

