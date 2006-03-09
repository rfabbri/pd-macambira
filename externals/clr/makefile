current: cygwin

clean: ; rm -f *.dll *.o


cygwin: clr.dll	

.SUFFIXES: .dll


# where are the PD header files?
# leave it blank if it is a system directory (like /usr/local/include), 
# 	since gcc 3.2 complains about it

PDPATH=/cygdrive/c/data/pd/pd-cvs
MONOPATH=/cygdrive/c/programme/prog/mono


CFLAGS = -DPD -O2 -mno-cygwin

PDINC= -I$(PDPATH)/src
PDLIB= $(PDPATH)/bin/pd.dll

MONOINC = -I$(MONOPATH)/include -I$(MONOPATH)/include/glib-2.0 -I$(MONOPATH)/lib/glib-2.0/include
MONOLIB = -L$(MONOPATH)/lib -lmono.dll -lgmodule-2.0 -lgthread-2.0 -lglib-2.0 -lintl -liconv

.cpp.dll:
	g++ $(CFLAGS) $(PDINC) $(MONOINC) -o $*.o -c $*.cpp
	g++ -export_dynamic  -shared -mno-cygwin $(PDLIB) $(MONOLIB) -o $*.dll $*.o -lm
#	ld -export_dynamic $(MONOLIB) -o $*.dll $*.o -lm pd.dll
#	strip --strip-unneeded $*.pd_linux
#	rm $*.o
