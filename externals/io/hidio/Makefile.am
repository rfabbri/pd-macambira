AUTOMAKE_OPTIONS = foreign

noinst_PROGRAMS = hidio.@EXTENSION@
SOURCES = hidio.c hidio_types.c
LDADD = 

noinst_HEADERS = hidio.h input_arrays.h

EXTRA_SUBDIRS = 

if MACOSX
SOURCES += hidio_darwin.c
EXTRA_SUBDIRS += HID_Utilities_Source
LDADD += HID_Utilities_Source/libHIDUtilities.a
CFLAGS += -IHID_Utilities_Source
LDFLAGS += -bundle -undefined dynamic_lookup \
	-framework Carbon -framework IOKit -framework ForceFeedback
endif

if LINUX
SOURCES += hidio_linux.c input_arrays.c
endif

if WINDOWS
SOURCES += hidio_windows.c
endif

# files that are included but not built
EXTRA_DIST = COPYING README TODO

# subdirs that are built
SUBDIRS = $(EXTRA_SUBDIRS)

# subdirs that are always included in the dist, etc.
DIST_SUBDIRS = HID_Utilities_Source

