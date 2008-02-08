CC	= cc
CFLAGS	= -Wmost
LDFLAGS = -framework Carbon -framework CoreMIDI
OBJS	= ptdarwin.o pmutil.o pmmacosx.o pmdarwin.o portmidi.o
LIBS	=

all: libportmidi.a pmtest

libportmidi.a: portmidi.h porttime.h pminternal.h $(OBJS)
	rm -f libportmidi.a
	ar rv libportmidi.a $(OBJS)
	ranlib libportmidi.a	

pmtest: pmtest.c libportmidi.a
	$(CC) $(CFLAGS) pmtest.c $(OBJS) -o pmtest $(LDFLAGS) $(LIBS)

pmmacosx.o: pmmacosx.c portmidi.h pminternal.h pmmacosx.h porttime.h
pmdarwin.o: pmdarwin.c portmidi.h pmmacosx.h
pmutil.o: pmutil.c portmidi.h pmutil.h pminternal.h
portmidi.o: portmidi.c portmidi.h pminternal.h
ptdarwin.o: ptdarwin.c porttime.h portmidi.h

clean:
	rm -f pmtest *.o
