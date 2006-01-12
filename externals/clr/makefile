current: cygwin

clean: ; rm -f *.dll *.o


cygwin: clr.dll	

.SUFFIXES: .dll


# where are the PD header files?
# leave it blank if it is a system directory (like /usr/local/include), 
# 	since gcc 3.2 complains about it
PDPATH=/home/Davide

LINUXCFLAGS = -DPD -O2 -mno-cygwin
#LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
#    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
#    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I../../src -I$PDPATH

MONOSRC = -IC:/cygwin/tmp/build_deps/include -IC:/cygwin/tmp/build_deps/include/glib-2.0 -IC:/cygwin/tmp/build_deps/lib/glib-2.0/include -IC:/cygwin/home/Davide/mono/include 
MONOLIB = -LC:/cygwin/tmp/build_deps/lib -LC:/cygwin/home/Davide/mono/lib -lmono -lm -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl -liconv 

.c.dll:
#	gcc $(LINUXCFLAGS) $(LINUXINCLUDE) `pkg-config --cflags --libs mono` -lm -ID:/Davide/cygwin/home/Davide/mono/include -LD:/Davide/cygwin/home/Davide/mono/lib $*.c
	gcc $(LINUXCFLAGS) $(LINUXINCLUDE) $(MONOSRC) -o $*.o -c $*.c
	gcc -export_dynamic  -shared -mno-cygwin $(MONOLIB) -o $*.dll $*.o -lm pd.dll libmono.dll.a
#	ld -export_dynamic $(MONOLIB) -o $*.dll $*.o -lm pd.dll
#	strip --strip-unneeded $*.pd_linux
#	rm $*.o
