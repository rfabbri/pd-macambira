NAME=pbank
CSYM=pbank


current: echo make pd_nt, pd_darwin, pd_linux

# ----------------------- NT -----------------------


pd_nt: pbank.dll

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo
VC="C:\Program Files\Microsoft Visual Studio\Vc98"

PDNTINCLUDE = /I. /I\tcl\include E:\pds\win32\pd37\src /I$(VC)\include

PDNTLDIR = $(VC)\lib
PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	..\..\..\..\pd\bin\pd.lib 

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$*_setup $*.obj $(PDNTLIB)
# ----------------------- IRIX 5.x -----------------------

pd_irix5: $(NAME).pd_irix5

.SUFFIXES: .pd_irix5

SGICFLAGS5 = -o32 -DPD -DUNIX -DIRIX -O2

SGIINCLUDE =  -I../../src

.c.pd_irix5:
	cc $(SGICFLAGS5) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -elf -shared -rdata_shared -o $*.pd_irix5 $*.o
	rm $*.o

# ----------------------- IRIX 6.x -----------------------

pd_irix6: $(NAME).pd_irix6

.SUFFIXES: .pd_irix6

SGICFLAGS6 = -n32 -DPD -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-Ofast=ip32

.c.pd_irix6:
	cc $(SGICFLAGS6) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -n32 -IPA -shared -rdata_shared -o $*.pd_irix6 $*.o
	rm $*.o

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
LINUXINCLUDE =  -I../../src -I../../src/pdinclude 

.c.pd_linux:
	$(CC) -O2 -Wall -DPD -fPIC $(NS_INCLUDE) -c $*.c 
	$(LD) -export_dynamic  -shared -o $*.pd_linux $*.o -lc 
	rm -f $*.o ../$*.pd_linux 
	#cp $*.pd_linux ../../xjimmies/.

# ----------------------- Mac OS X (Darwin) -----------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# points to where your m_pd.h: either in your pd sources,  or in the borrowed (older) pd ".h" files
DARWININCLUDE =   -I../../../pd/src  -I../pdinclude

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(NS_INCLUDE) -o $*.o -c $*.c
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o
	rm -f $*.o ../$*.pd_darwin
	#cp $*.pd_darwin ../../xjimmies/.
	#ln -s $*/$*.pd_darwin ..


# ----------------------------------------------------------


clean:
	rm -f *.o *.pd_* so_locations
