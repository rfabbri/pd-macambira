OBJ = carules.scafo

SCAFDIR = ../system

.SUFFIXES: .scaf
.SUFFIXES: .scafo

.scaf.o:
	$(SCAFDIR)/scafc.pl -I$(SCAFDIR) $*.scaf | as -o $*.o

.o.scafo:
	gcc -export_dynamic -shared -o $*.scafo $*.o
	rm $*.o
	strip --strip-unneeded $*.scafo

all:	$(OBJ)

clean:
	rm -f *.scafo
	rm -f *~

