PD = /home/matju/pd_0_38_4/bin/pd -noaudio
#PD = pd
PDFLAGS = -lib pureunity -noaudio -path generics -path specifics

test:: built
	$(PD) $(PDFLAGS) main.pd

built: Makefile pureunity.pd_linux
	# for z in associator commutator distributor invertor; do sed s/inlet/inlet~/ < generics/$$z.pd > generics/$$z~.pd; done
	echo > built

generic/%~.pd: generic/%.pd

pureunity.pd_linux: pureunity.c Makefile
	$(CC) -shared -o pureunity.pd_linux pureunity.c

