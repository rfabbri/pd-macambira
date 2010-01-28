AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = hidio
hidio_SOURCES = hidio.c hidio_types.c
hidio_CFLAGS = $(CFLAGS)
hidio_LDADD = 
hidio_LDFLAGS = $(LDFLAGS)
noinst_HEADERS = hidio.h input_arrays.h

EXTRA_SUBDIRS = 

if MACOSX
hidio_SOURCES += hidio_darwin.c
EXTRA_SUBDIRS += HID_Utilities_Source
hidio_CFLAGS += -IHID_Utilities_Source
hidio_LDADD += HID_Utilities_Source/libHIDUtilities.a
hidio_LDFLAGS += -bundle -undefined dynamic_lookup \
	-framework Carbon -framework IOKit -framework ForceFeedback
endif

if LINUX
hidio_SOURCES += hidio_linux.c input_arrays.c
endif

if WINDOWS
hidio_SOURCES += hidio_windows.c
endif



# files that are included but not built
EXTRA_DIST = COPYING README TODO

# subdirs that are built
SUBDIRS = $(EXTRA_SUBDIRS)

# subdirs that are always included in the dist, etc.
DIST_SUBDIRS = HID_Utilities_Source

