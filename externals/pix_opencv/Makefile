# To use this Makefile for your project, first put the name of your library in
# LIBRARY_NAME variable. The folder for your project should have the same name
# as your library.
LIBRARY_NAME = pix_opencv
LIBRARY_VERSION = 0.2

# Next, add your source files to the SOURCES variable.
SOURCES = pix_opencv_edge.cc pix_opencv_laplace.cc pix_opencv_morphology.cc pix_opencv_distrans.cc pix_opencv_motempl.cc pix_opencv_haarcascade.cc pix_opencv_contours_boundingrect.cc pix_opencv_bgsubstract.cc pix_opencv_contours_convexity.cc pix_opencv_dft.cc pix_opencv_lk.cc pix_opencv_hist_compare.cc pix_opencv_knear.cc pix_opencv_threshold.cc pix_opencv_floodfill.cc pix_opencv_athreshold.cc pix_opencv_bgstats.cc pix_opencv_camshift.cc pix_opencv_hu_compare.cc pix_opencv_pgh_compare.cc pix_opencv_hough_circles.cc pix_opencv_hough_lines.cc pix_opencv_hu_moments.cc pix_opencv_contours_convexhull.cc pix_opencv_colorfilt.cc pix_opencv_surf.cc

# For objects that only build on certain platforms, add those to the SOURCES
# line for the right platforms.
SOURCES_Darwin = 
SOURCES_Linux = 
SOURCES_Windows = 

#------------------------------------------------------------------------------#
#
# you shouldn't need to edit anything below here, if we did it right :)
#
#------------------------------------------------------------------------------#

# where Pd lives
PD_PATH = ../../pd
GEM_PATH = ../../Gem
# where to install the library
objectsdir = $(PD_PATH)/extra

CFLAGS = -DPD -I$(PD_PATH)/src -I$(GEM_PATH)/src -Wall -W -g
LDFLAGS =  
LIBS = 

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  SOURCES += $(SOURCES_Darwin)
  EXTENSION = pd_darwin
  OS = macosx
  OPT_CFLAGS = -fast
  FAT_FLAGS = -arch i386 -arch ppc -mmacosx-version-min=10.4
  CFLAGS += -fPIC -F.
  LDFLAGS += -bundle -undefined dynamic_lookup -F.
  LIBS += -lc -framework OpenCV
  STRIP = strip -x
 endif
ifeq ($(UNAME),Linux)
  SOURCES += $(SOURCES_Linux)
  EXTENSION = pd_linux
  OS = linux
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  CFLAGS += -fPIC
  LDFLAGS += -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  SOURCES += $(SOURCES_Windows)
  EXTENSION = dll
  OS = windows
  OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer -march=i686 -mtune=pentium4
  WINDOWS_HACKS = -D'O_NONBLOCK=1'
  CFLAGS += -mms-bitfields $(WINDOWS_HACKS)
  LDFLAGS += -s -shared -Wl,--enable-auto-import
  LIBS += -L$(PD_PATH)/src -L$(PD_PATH)/bin -L$(PD_PATH)/obj -lpd -lwsock32 -lkernel32 -luser32 -lgdi32
  STRIP = strip --strip-unneeded -R .note -R .comment
endif

CFLAGS += $(OPT_CFLAGS)


.PHONY = install libdir_install single_install install-doc install-exec clean dist etags

all: $(SOURCES:.cc=.$(EXTENSION))

%.$(EXTENSION): %.cc
	$(CXX) $(CFLAGS) -o "$*.o" -c "$*.cc"
	$(CC) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o"  $(LIBS)
	chmod a-x "$*.$(EXTENSION)"
	rm -f -- $*.o


install: libdir_install

# The meta and help files are explicitly installed to make sure they are
# actually there.  Those files are not optional, then need to be there.
libdir_install: $(SOURCES:.cc=.$(EXTENSION)) install-doc install-exec
	install -d $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	install -m644 -p $(LIBRARY_NAME)-meta.pd $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	install -m644 -p $(SOURCES:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(STRIP) $(addprefix $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/,$(SOURCES:.cc=.$(EXTENSION)))

# install library linked as single binary
single_install: $(LIBRARY_NAME) install-doc install-exec
	install -d $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	install -m644 -p $(LIBRARY_NAME).$(EXTENSION) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(STRIP) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/$(LIBRARY_NAME).$(EXTENSION)

install-doc:
	install -d $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
#	install -m644 -p $(SOURCES:.c=-help.pd) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	install -m644 -p $(wildcard *.pd) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	install -m644 -p README $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/README.txt
	install -m644 -p VERSION $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/VERSION.txt
	install -m644 -p CHANGES $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/CHANGES.txt

install-exec:
	install -d $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	install -m644 -p $(wildcard *.pd) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)


clean:
	-rm -f -- $(SOURCES:.cc=.o)
	-rm -f -- $(SOURCES:.cc=.$(EXTENSION))
	-rm -f -- $(LIBRARY_NAME).$(EXTENSION)

distclean: clean
	-rm -f -- ../$(LIBRARY_NAME)-$(OS)-$(shell uname -m).tar.bz2
	-rm -f -- ../$(LIBRARY_NAME)-$(OS).tar.bz2

dist: all dist_$(OS)

dist_linux:
	cd .. && tar --exclude=.svn -cjpf $(LIBRARY_NAME)-$(OS)-$(shell uname -m).tar.bz2 $(LIBRARY_NAME)

dist_macosx:
	cd .. && tar --exclude=.svn -cjpf $(LIBRARY_NAME)-$(OS).tar.bz2 $(LIBRARY_NAME)

dist_windows:
	cd .. && tar --exclude=.svn -cjpf $(LIBRARY_NAME)-$(OS).tar.bz2 $(LIBRARY_NAME)


etags:
	etags *.[ch] ../../pd/src/*.[ch] /usr/include/*.h /usr/include/*/*.h

showpaths:
	@echo "PD_PATH: $(PD_PATH)"
	@echo "objectsdir: $(objectsdir)"
	@echo "LIBRARY_NAME: $(LIBRARY_NAME)"
	@echo "SOURCES: $(SOURCES)"
