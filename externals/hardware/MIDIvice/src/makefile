current: all


# the MIDIVICE-EXTERNAL-makefile
# everything is GnuGPL that should come with the MIDIvice.tgz
# NO WARRANTIES FOR ANYTHING
# et cetera
# 1999:forum::für::umläute:2001

# make sure that the "m_pd.h" is somehow available either by putting it into this
# directory, by adding it's path to the INCLUDE-path or by putting it into an
# already included path, e.g. "/usr/local/include/"

#these are the user adjustables : adjust them to fit into your system
# PD will install to $(DESTDIR)$(INSTALLL_PREFIX)$(PDLIBDIR), which is /usr/local/lib/pd
# by default
DESTDIR =
INSTALL_PREFIX = /usr/local
PDLIBDIR = /lib/pd
#these were the user adjustables


TARGETS = MIDIvice \
	motormix

# ----------------------- LINUX ----------------------------
.SUFFIXES: .pd_linux


LINUXOBJECTS = $(TARGETS:%=%.o)
ARCH = $(shell uname --machine)

PD_DIR = $(DESTDIR)$(INSTALL_PREFIX)$(PDLIBDIR)

ifeq (${ARCH},alpha)
AFLAGS = -mieee  -mcpu=ev56
endif

LINCLUDE =

$(LINUXOBJECTS): *.h

CFLAGS = -O2 -g -Wall $(LINCLUDE) $(UCFLAGS) $(AFLAGS)

everything: clean all install distclean

distclean:
	touch dummy.o
	touch dummy.pd_linux
	touch dummy~
	touch _dummy
	rm *.o *.pd_linux *~ _*

clean:
	touch dummy.o
	touch dummy.pd_linux
	rm *.o *.pd_linux

all: $(LINUXOBJECTS)

	@echo :: $(LINUXOBJECTS)

	ld -export_dynamic  -shared -o MIDIvice.pd_linux *.o -lc -lm
	strip --strip-unneeded MIDIvice.pd_linux

.c.pd_linux:
	cc $(CFLAGS) -O2 -DPD -fPIC $(INCLUDE) -c -o $*.o $*.c


install: installdocs
	install -m 644 MIDIvice.pd_linux $(PD_DIR)/extra

installdocs:
	install -d $(PD_DIR)/doc/5.reference/MIDIvice
	install -m644 ../doc/* $(PD_DIR)/doc/5.reference/MIDIvice
