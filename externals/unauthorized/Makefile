EXT=pd_darwin

INSTALL_PREFIX=/usr/local
CC=gcc

# find all files to compile
TARGETS=$(subst .tk,.tk2c,$(wildcard *.tk)) $(subst .c,.$(EXT),$(wildcard *.c))

current: $(EXT)

.SUFFIXES: .pd_linux .pd_darwin .pd_irix5 .pd_irix6 .dll .tk .tk2c

# ----------------------- prep -----------------------
# copy all of the files into the root dir for compiling

prep:
	cp */*.c */*.h */*.cc */*.tk .
	rm grid*.* mp*.*

# ----------------------- Mac OSX -----------------------

pd_darwin: $(TARGETS)


DARWINCFLAGS = -DPD -DUNIX -DMACOSX -DICECAST -O2 -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

.c.osx:
	$(CC) $(DARWINCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c

.tk.tk2c:
	./tk2c.bash < $*.tk > $*.tk2c

.c.pd_darwin:
	$(CC) $(DARWINCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	$(CC) -bundle -undefined suppress  -flat_namespace -o $*.pd_darwin $*.o
	-rm $*.o

# ----------------------- NT -----------------------

pd_nt: prep $(NAME).dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo
VC="C:\Program Files\Microsoft Visual Studio\Vc98"

PDNTINCLUDE = /I. /I\tcl\include /I\ftp\pd\src /I$(VC)\include

PDNTLDIR = $(VC)\lib
PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	\ftp\pd\bin\pd.lib 

.c.ont:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
# need to find a way to replace $(CSYM)
#	link /dll /export:$(CSYM)_setup $*.obj $(PDNTLIB)

# ----------------------- IRIX 5.x -----------------------

pd_irix5: prep $(NAME).pd_irix5 

SGICFLAGS5 = -o32 -DPD -DUNIX -DIRIX -O2

SGIINCLUDE =  -I../../src

.c.oi5:
	$(CC) $(SGICFLAGS5) $(SGIINCLUDE) -o $*.o -c $*.c

.c.pd_irix5:
	$(CC) $(SGICFLAGS5) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -elf -shared -rdata_shared -o $*.pd_irix5 $*.o
	rm $*.o

# ----------------------- IRIX 6.x -----------------------

pd_irix6: prep $(NAME).pd_irix6

SGICFLAGS6 = -n32 -DPD -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-Ofast=ip32

.c.oi6:
	$(CC) $(SGICFLAGS6) $(SGIINCLUDE) -o $*.o -c $*.c

.c.pd_irix6:
	$(CC) $(SGICFLAGS6) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -n32 -IPA -shared -rdata_shared -o $*.pd_irix6 $*.o
	rm $*.o

# ----------------------- LINUX i386 -----------------------

pd_linux: prep $(NAME).pd_linux

LINUXCFLAGS = -DPD -DUNIX -DICECAST -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wno-shadow -Wstrict-prototypes -g \
    -Wno-unused -Wno-parentheses -Wno-switch -Werror

LINUXINCLUDE =  -I../../src -I/usr/local/src/pd/src

.c.o:
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c

.c.pd_linux:
	./tk2c.bash < $*.tk > $*.tk2c
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o

# ----------------------------------------------------------

install:
	cp */*.pd ../../doc/5.reference

clean:
	-rm -f *.?~ *.o *.pd_* *.dll *.tk2c core so_locations
	-rm -f ./*.c ./*.h ./*.cc ./*.tk
