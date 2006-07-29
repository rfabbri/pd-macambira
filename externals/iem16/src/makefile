current: all

# the IEM16-EXTERNAL-makefile
# everything is GnuGPL that should come with the iem16.tgz
# NO WARRANTIES FOR ANYTHING
# et cetera
# forum::für::umläute@IEM:2003

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


TARGETS = iem16 \
	iem16_table \
	iem16_array iem16_array_tilde \
	iem16_delay

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

#CFLAGS = -O2 -g -Wall $(LINCLUDE) $(UCFLAGS) $(AFLAGS)
CFLAGS = -fPIC -O3 -g -Wall $(LINCLUDE) $(UCFLAGS) $(AFLAGS)


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

	ld -export_dynamic  -shared -o iem16.pd_linux *.o -lc -lm
	strip --strip-unneeded iem16.pd_linux

.c.pd_linux:
	cc $(CFLAGS) -O2 -DPD -fPIC $(INCLUDE) -c -o $*.o $*.c


install: installdocs
	install -d $(PD_DIR)/extra
	install -m 644 iem16.pd_linux $(PD_DIR)/extra

installdocs:
	install -d $(PD_DIR)/doc/5.reference/iem16
	install -m644 ../examples/*.pd $(PD_DIR)/doc/5.reference/iem16
