current: pd_linux
###############################
NAME=OSC
EXT=o
LIBS = -lm -lc
LIBOSC = 	../libOSC/libOSC.a
DEFS= -Dunix

# ----------------------- Mac OS X (Darwin) -----------------------

pd_darwin: $(NAME).pd_darwin

SFX=.pd_darwin

.SUFFIXES: $(SFX)

DARWINCFLAGS = -DPD -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
DARWININCLUDE =  -I../../../pd/src

DARWINEXTERNALS = htmsocket.o OSC-pattern-match.o sendOSC.o dumpOSC.o routeOSC.o

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c *.c
	cc -bundle -bundle_loader /usr/local/pd/bin/pd -flat_namespace -o $*.pd_darwin $*.o $(DARWINEXTERNALS) $(LIBS) $(LIBOSC)

	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..


# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

SFX=pd_linux

.SUFFIXES: .$(SFX)

LINUXCFLAGS = -DPD -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
LINUXINCLUDE =  -I../../build/include

LINUXEXTERNALS = htmsocket.o OSC-pattern-match.o sendOSC.o dumpOSC.o routeOSC.o

.c.pd_linux:
	cc -O2 -Wall -DPD -fPIC $(LINUXCFLAGS) $(LINUXINCLUDE) -c *.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o $(LINUXEXTERNALS) $(LIBS)  $(LIBOSC)
	strip --strip-unneeded $*.pd_linux

# ----------------------------------------------------------

pd_linux_clean:
	    -rm -rf *.$(EXT) *.pd_linux *~

pd_darwin_clean:
	    -rm -rf *.$(EXT) *.pd_darwin *~

