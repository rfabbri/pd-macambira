LIBOSCDIR = ../libOSC
LIBOSC = ${LIBOSCDIR}/libOSC.a
# for eg. linux
# DEFS= -Dunix
# for MAC OSX, should be DARWIN
DEFS=-Dunix -DMACOSX

CFLAGS=  -O2 -I$(LIBOSCDIR) $(DEFS) -Wall

DUMPOBJS=dumpOSC.o


both: sendOSC dumpOSC

sendOSC: sendOSC.o htmsocket.o ${LIBOSC}
	${CC} ${CFLAGS} ${DEFS} -o sendOSC sendOSC.o htmsocket.o  ${LIBOSC}

dumpOSC: ${DUMPOBJS}
	${CC} ${CFLAGS} ${DEFS} -o $@ ${DUMPOBJS}
	
dumpUDP: dumpUDP.o
	${CC} ${CFLAGS} ${DEFS} -o dumpUDP dumpUDP.o

${LIBOSC}:
	echo "You need to go to " ${LIBOSCDIR} " and do a make."
	(cd ../libOSC ; make)

clean:
	rm -f sendOSC dumpOSC *.o


