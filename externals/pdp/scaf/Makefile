include Makefile.config

all: pdp_scaf.pd_linux

pdp_scaf_all:
	make -C include
	make -C system
	make -C modules
	make -C pdp

clean:
	rm -f *~
	rm -f pdp_scaf.pd_linux
	make -C include clean
	make -C system clean
	make -C modules clean
	make -C pdp clean


pdp_scaf.pd_linux: pdp_scaf_all
	rm -f pdp_scaf.pd_linux
	gcc -export_dynamic -shared -o pdp_scaf.pd_linux pdp/*.o system/scaf_feeder.o $(PDP_CA_LIBS)
