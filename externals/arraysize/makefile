# just the build-commands

I =  -I../../../bin/pd/src

all: linux

linux:
	cc -DPD -O2 -DUNIX $(I) -o arraysize.o -c arraysize.c
	ld -shared -export_dynamic -o arraysize.pd_linux arraysize.o

nt:
	cl arraysize.c /DNT /DPD /LD /Gd /GD /Ox /link /export:arraysize_setup \system\pd\bin\pd.lib
