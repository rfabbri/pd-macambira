
# Adapt the PDPATH if your pd is not installed in the standard place

PDPATH="$(PROGRAMFILES)/pd"




EXTERNALS = $(shell ls ../src)

all:
	make -k externals

externals: $(EXTERNALS:.c=.dll)

.SUFFIXES: .dll

DEFINES = -DPD -DNT 
CFLAGS = -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

INCLUDE = -I. -I.. -I$(PDPATH)/src


%.dll: ../src/%.c
	$(CC) -mms-bitfields $(CFLAGS) $(DEFINES) $(INCLUDE) -o "$*.o" -c "../src/$*.c"
	gcc -shared -o "$*.dll" "$*.o" $(PDPATH)/bin/pd.dll \
		`test -f $*.libs && cat $*.libs` `test -f ../src/$*.libs && cat ../src/$*.libs` 

clean:
	-rm *.a *.def *.dll *.o

install-doc:
	@test -d $(PDPATH)/doc/5.reference || mkdir -p $(PDPATH)/doc/5.reference
	cd ../doc && make all
	cp -r ../doc/* $(PDPATH)/doc/5.reference

install: install-doc
	@test -d $(PDPATH)/extra || mkdir -p $(PDPATH)/extra
	install *.dll $(PDPATH)/extra
