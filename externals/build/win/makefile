# ----------------------- NT -----------------------


EXTERNALS=\
#!IF ![f exist filelist.inc del filelist.inc & for %i in (*.c) do @echo %i \>> filelist.inc]
!INCLUDE filelist.inc
#!ENDIF

all: filelist.inc link.stamp $(EXTERNALS:.c=.dll)

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo
VC="C:\Program Files\Microsoft Visual Studio\Vc98"
OGGPATH="C:\oggvorbis-win32sdk-1.0"
SRCPATH=..\src
PDPATH=..\..\..\pd

PDNTINCLUDE = /I. /I.. /I..\..\..\pd\src /I$(VC)\include \
	/I..\..\creb\include /I$(OGGPATH)/include

PDNTLDIR = $(VC)\lib
PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\uuid.lib \
	$(PDNTLDIR)\ws2_32.lib \
	$(PDPATH)\bin\pthreadVC.lib \
	$(PDPATH)\bin\pd.lib \
	$(OGGPATH)\lib\vorbis.lib \
	$(OGGPATH)\lib\vorbisenc.lib \
	$(OGGPATH)\lib\vorbisfile.lib \
	$(OGGPATH)\lib\ogg.lib

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$(*:~=_tilde)_setup $(*).obj $(PDNTLIB)
	del $*.obj $*.lib $*.exp

filelist.inc:
	if exist filelist.inc del filelist.inc
#	for %i in ($(SRCPATH)\*.c) do @echo %~ni%~xi \>> filelist.inc
	for %i in ($(SRCPATH)\*.c) do @echo %~ni%~xi \>> filelist.inc

transfer:
	copy $(SRCPATH)\*.c .

link.stamp: filelist.inc
	copy $(SRCPATH)\*.c .
	copy /y nul link.stamp
	nmake

clean:
# don't delete filelist.inc at the moment, as some externals can't be compiled
# on win32
	-del link.stamp
	-del *.c	
	-del *.obj
	-del *.lib
	-del *.exp
	-del *.dll
