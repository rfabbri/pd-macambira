NAME=chaos
CSYM=chaos

current: pd_nt pd_intel pd_linux pd_darwin

# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /MD /O2 /G6 /DNT /DPD /DMAXLIB /nologo
VC="C:\Programme\Microsoft Visual Studio\VC98"

PDNTINCLUDE = /I. /Ic:\pd\tcl\include /Ic:\pd\src /I$(VC)\include /Iinclude

PDNTLDIR = $(VC)\Lib
PDNTLIB = $(PDNTLDIR)\msvcrt.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\user32.lib \
	$(PDNTLDIR)\uuid.lib \
	$(PDNTLDIR)\ws2_32.lib \
	$(PDNTLDIR)\pthreadVC.lib \
	c:\pd\bin\pd.lib
        
PDNTEXTERNALS = lorenz.obj rossler.obj henon.obj ikeda.obj

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c *.c
	link /dll /export:$(CSYM)_setup $*.obj $(PDNTEXTERNALS) $(PDNTLIB)

# ----------------------- Mac OS X (Darwin) -----------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DMAXLIB -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
DARWININCLUDE =  -I../../src -I../../obj

DARWINEXTERNALS = lorenz.o rossler.o henon.o ikeda.o

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

LINUXEXTERNALS = lorenz.o rossler.o henon.o ikeda.o

.c.pd_linux:
	cc -O2 -Wall -DPD -fPIC $(LINUXCFLAGS) $(LINUXINCLUDE) -c *.c 
	ld -export_dynamic  -shared -o $*.pd_linux $*.o $(LINUXEXTERNALS) -lc 
	strip --strip-unneeded $*.pd_linux 

# ----------------------------------------------------------

PDDIR=/usr/lib/pd

clean:
	rm -f *.o *.pd_* so_locations
