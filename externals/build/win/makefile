prefix=../../..

EXTERNALS = $(shell ls ../src)

all: $(EXTERNALS:.c=.dll)

.SUFFIXES: .dll

CFLAGS = -DPD -DMSW -DNT -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

INCLUDE = -I. -I.. -I ../../../pd/src -I../include -I/usr/local/include

%.dll: ../src/%.c
	$(CC) $(CFLAGS) $(INCLUDE) -o "$*.o" -c "../src/$*.c"
	gcc -shared -o "$*.dll" "$*.o" $(prefix)/bin/pd.dll \
		`test -f $*.libs && cat $*.libs` -L/usr/local/lib \
		`test -f ../src/$*.libs && cat ../src/$*.libs`
clean:
	-rm *.a *.def *.dll *.o

install-doc:
	@test -d $(prefix)/doc/5.reference || mkdir -p $(prefix)/doc/5.reference
	cd ../doc && make all
	cp -r ../doc/* $(prefix)/doc/5.reference

install: install-doc
	@test -d $(prefix)/extra || mkdir -p $(prefix)/extra
	install *.dll $(prefix)/extra
