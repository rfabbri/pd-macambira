all: tracker.pd_linux
tracker.pd_linux: tracker.c tracker.h tracker.tk2c

CFLAGS = -DPD -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wno-shadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch \
	-I/usr/include -I../pd/src

.SUFFIXES: .pd_linux .tk .tk2c

.tk.tk2c:
	sh tk2c.bash < $*.tk >$*.tk2c
.c.pd_linux:
	$(CC) $(CFLAGS) -o $*.o -c $*.c
	$(LD) -export_dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
clean:
	rm -f *.o *.pd_* so_locations *.tk2c
