
current: linux

.SUFFIXES: .pd_linux
INSTALL_PREFIX = /usr
PD_DIR = $(INSTALL_PREFIX)/lib/pd

VERSION = \"$(shell cat VERSION)\"
#LIBOSC = 	../OSC/perl/libOSC/libOSC.a
LIBOSC = 

TARGETS = binshift ascwave ixprint bangfilt bangfilt2 \
	counter reson prepend microtime ascseq \
	randomix ENV proc mean sigdelta cx

# mean
# serialize 

LINUXOBJECTS = $(TARGETS:%=%.o)

LINCLUDE = -I. -I../pd-0.35-test24/src
# possible defines
# -DPRINTADDRS
DEBUG = -DDEBUG -ggdb
#DEBUG =
CFLAGS =  -O2 -DPD -DVERSION=$(VERSION) $(DEBUG) -fPIC  $(LINCLUDE) 
LIBS = -lc -lm 
linux: $(LINUXOBJECTS)
	ld -export_dynamic  -shared -o cxc.pd_linux *.o $(LIBS) ${LIBOSC}
	strip --strip-unneeded cxc.pd_linux

.c.pd_linux:
	cc $(CFLAGS) -c $*.c ${LIBOSC}

clean:
	rm *.pd_linux *.o *~

install:
	install -d $(PD_DIR)/doc/5.reference/cxc
	install -m664  reference/*.pd $(PD_DIR)/doc/5.reference/cxc
	install -m664 cxc.pd_linux $(PD_DIR)/externs
uninstall:
	rm -rf $(PD_DIR)/doc/5.reference/cxc
	rm -rf $(PD_DIR)/externs/cxc.pd_linux
