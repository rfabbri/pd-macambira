NAME=wiimote

# If you want to use a customized Pd, then define a $PD_PATH variable.
# Otherwise, the Pd must be installed on the system
PD_PATH=../../../pd

# specify the path to CWiid:
#CWIID_PATH = $(ASCAPE_PATH)/usr/lib
LIBCWIID_PATH = "cwiid-svn/libcwiid"

######################################################
# You shouldn't need to change anything beyond here! #
######################################################


ifdef PD_PATH
PD_INCLUDE := -I$(PD_PATH)/src
PD_EXTRA_PATH := ../../../../lib/libs
PD_DOC_PATH := ../../../../lib/pd-help
else
PD_INCLUDE := -I../../../pd/src
PD_EXTRA_PATH := /usr/local/lib/pd/extra
PD_DOC_PATH := /usr/local/lib/pd/doc
endif


#LIBS = $(LIBCWIID_PATH)/libcwiid.a -lbluetooth -lpthread
LIBS = -lcwiid -lbluetooth -lpthread

current: pd_linux

##### LINUX:

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

.c.pd_linux:
	$(CC) $(LINUXCFLAGS) $(PD_INCLUDE) -o $*.o -c $*.c
	$(LD) --export-dynamic -shared -o $*.pd_linux $*.o $(LIBS) -lc -lm
	strip --strip-unneeded $*.pd_linux 
	rm -f $*.o

install:

ifdef ASCAPE_INSTALLED
	-cp *help*.pd $(ASCAPE_PATH)/ss_engine/pd/help/.
ifeq ($(findstring Linux,$(ASCAPE_OS)),Linux)
	-cp *.pd_linux $(ASCAPE_PATH)/ss_engine/pd/externs/$(ASCAPE_OS)$(ASCAPE_ARCH)/.
endif
ifeq ($(findstring Darwin,$(SS_OS)),Darwin)
	-cp *.pd_darwin $(ASCAPE_PATH)/ss_engine/pd/externs/$(ASCAPE_OS)$(ASCAPE_ARCH)/.
endif
endif

	-cp *.pd_linux $(PD_EXTRA_PATH)/.
	-cp *help*.pd $(PD_DOC_PATH)/.

clean:
	-rm -f *.o *.pd_* so_locations
