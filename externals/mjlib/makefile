all: mjLib

VC = "C:\Program Files\Microsoft Visual Studio .NET\Vc7"
INCLUDE = -I. -I..\src \
-I..\Tcl\include -I$(VC)\include -I"C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\Include"

LDIR = $(VC)\lib
LDIR2 = "C:\Program Files\Microsoft Visual Studio .NET\Vc7\PlatformSDK\lib"

LIB = /NODEFAULTLIB:libc /NODEFAULTLIB:oldnames  /NODEFAULTLIB:kernel \
    /NODEFAULTLIB:uuid \
    $(LDIR)\libc.lib $(LDIR)\oldnames.lib $(LDIR)\kernel32.lib \
    $(LDIR2)\wsock32.lib $(LDIR2)\winmm.lib \
    ..\bin\pd.lib
GLIB =  $(LIB) ..\lib\tcl83.lib ..\lib\tk83.lib
CFLAGS = /nologo /W3 /WX /DNT /DPD  /Ox /Zi /DVERSION=\"1\" 
LFLAGS = /nologo

SRC = pin~.c mjLib.c  metroplus.c monorythm.c prob.c about.c synapseA~.c convolve~.c n2m.c morse.c

OBJ = $(SRC:.c=.obj)

.c.obj:
	cl /c $(CFLAGS) $(INCLUDE) $*.c
	

mjLib: ..\mjLib\mjLib.dll

..\mjLib\mjLib.dll ..\mjLib\mjLib.lib: $(OBJ)
	link $(LFLAGS) /debug /dll /export:mjLib_setup \
	/out:..\mjLib\mjLib.dll $(OBJ) $(LIB)


# the following should also clean up "bin" but it doesn't because "bin" holds
# precious stuff from elsewhere.
clean:
	del *.obj
