pd_linux: pan_gogins~.pd_linux rawmouse.pd_linux rawevent.pd_linux rawjoystick.pd_linux 

clean: ; rm -f *.pd_linux *.o

# ----------------------- LINUX i386 -----------------------

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DUNIX -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch -g

LINUXINCLUDE =  -I/usr/local/lib/pd/include -I../../pd/src

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o

