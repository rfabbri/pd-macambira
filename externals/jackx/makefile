jackx: jackx.c
	gcc $(CFLAGS) $(LINUXCFLAGS) $(LINUXINCLUDE) -o jackx.o -c jackx.c
	ld -export_dynamic  -shared -o jackx.pd_linux jackx.o -lc -lm
	strip --strip-unneeded jackx.pd_linux
	rm jackx.o


