current: pd_linux

pd_linux: pdp_guile.pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch # -Wshadow

LINUXINCLUDE =  -I../distro/pd/src -Idbot -I../include

.c.pd_linux:
	gcc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	gcc -export_dynamic  -shared -o $*.pd_linux $*.o -lm \
		-lguile #-lguile-ltdl -lqthreads -lpthread -lcrypt

	strip --strip-unneeded $*.pd_linux
	rm $*.o

clean:
	rm -f *.pd_linux
	rm -f *~

