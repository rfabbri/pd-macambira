# where is your m_pd.h ???

LINUXINCLUDE =  -I/usr/local/include




beatpipe: beatpipe.c
	gcc $(CFLAGS) $(LINUXCFLAGS) $(LINUXINCLUDE) -o beatpipe.o -c beatpipe.c
	ld -export_dynamic  -shared -o beatpipe.pd_linux beatpipe.o -lc -lm
	strip --strip-unneeded beatpipe.pd_linux
	rm beatpipe.o


install:

	cp help-beatpipe.pd /usr/local/lib/pd/doc/5.reference
	cp beatpipe.pd_linux /usr/local/lib/pd/externs


clean:
	rm beatpipe.pd_linux

