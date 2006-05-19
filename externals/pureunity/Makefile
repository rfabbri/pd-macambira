# if you don't want the standard pd, set the PD-variable before doing "make"
# e.g. "PD=/home/matju/pd_devel_0_39_und/bin/pd make"
PD ?= pd
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

