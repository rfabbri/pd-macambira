ff: ff.c
	gcc $(CFLAGS) $(LINUXCFLAGS) $(LINUXINCLUDE) -o ff.o -c ff.c
	ld -export_dynamic  -shared -o ff.pd_linux ff.o -lc -lm
	strip --strip-unneeded ff.pd_linux
	rm ff.o


