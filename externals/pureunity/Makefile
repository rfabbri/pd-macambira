#PD = /home/matju/pd_0_38_4/bin/pd
PD = /home/matju/pd_devel_0_39_und/bin/pd
#PD = pd
PDFLAGS = -lib pureunity -noaudio -path generics -path specifics
PDFLAGS += -lib gridflow
CFLAGS = -Wall

test:: built
	$(PD) $(PDFLAGS) main.pd

built: Makefile pureunity.pd_linux
	echo > built

generic/%~.pd: generic/%.pd

pureunity.pd_linux: pureunity.c Makefile
	$(CC) $(CFLAGS) -shared -o pureunity.pd_linux pureunity.c

