# comport PD External Unix(Linux)/Windows
#
#  IEM - Institute for Electronic Music and Acoustic, Graz
#
#  Author: Winfried Ritsch
#  Maintainer: Win
#
#  Licence: GPL - Gnu Public Licence


current: pd_nt
	echo make pd_linux, pd_nt, pd_irix5, or pd_irix6


# ----------------------- NT -----------------------
pd_nt: comport.dll bird.dll

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo /DWIN2000

VC="C:\Programme\Microsoft Visual Studio\Vc98"
PDROOT="C:\Programme\pd"

PDNTINCLUDE = /I. /I$(PDROOT)\tcl\include /I$(PDROOT)\src /I$(VC)\include

PDNTLDIR = $(VC)\lib
PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDROOT)\bin\pd.lib 

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$*_setup $*.obj $(PDNTLIB)

# ----------------------- IRIX 5.x -----------------------

pd_irix5: comport.pd_irix5 bird.pd_irix5

.SUFFIXES: .pd_irix5

SGICFLAGS5 = -o32 -DPD -DSGI -O2


SGIINCLUDE =  -I../../src

.c.pd_irix5:
	cc $(SGICFLAGS5) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -elf -shared -rdata_shared -o $*.pd_irix5 $*.o
	rm $*.o

# ----------------------- IRIX 6.x -----------------------

pd_irix6: comport.pd_irix6 bird.pd_irix6

.SUFFIXES: .pd_irix6

SGICFLAGS6 = -DPD -DSGI -n32 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-Ofast=ip32

SGICFLAGS5 = -DPD -O2 -DSGI

SGIINCLUDE =  -I/../../src

.c.pd_irix6:
	cc $(SGICFLAGS6) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -elf -shared -rdata_shared -o $*.pd_irix6 $*.o
	rm $*.o

# ----------------------- LINUX i386 -----------------------

pd_linux: comport.pd_linux bird.pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I../../src

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o


# ---------- TEST ----------

TESTCFLAGS = $(LINUXCFLAGS)
LIBS = -lc -lm


test: testser.o bird.o
	gcc -o test testser.o bird.o $(LIBS)

tester.o: testser.c
	gcc -c -o testser.o testser.c $(TESTCFLAGS)

bird.o: bird.c
	gcc -c -o bird.o bird.c $(TESTCFLAGS)
