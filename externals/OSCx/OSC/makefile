###############################
NAME=OSC

# ----------------------- Mac OS X (Darwin) -----------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
DARWININCLUDE =  -I../../../pd/src -I../../obj

DARWINEXTERNALS = htmsocket.o OSC-pattern-match.o sendOSC.o dumpOSC.o routeOSC.o
# lorenz.o rossler.o henon.o ikeda.o

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c *.c
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o $(DARWINEXTERNALS)
	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..


# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
LINUXINCLUDE =  -I/usr/local/include

LINUXEXTERNALS = htmsocket.o OSC-pattern-match.o sendOSC.o dumpOSC.o routeOSC.o

.c.pd_linux:
	cc -O2 -Wall -DPD -fPIC $(LINUXCFLAGS) $(LINUXINCLUDE) -c *.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o $(LINUXEXTERNALS) -lc
	strip --strip-unneeded $*.pd_linux

# ----------------------------------------------------------

