
prefix=$(DESTDIR)/usr

EXTERNALS = $(shell ls ../src)
STRIP = strip --strip-unneeded -R .note -R .comment

all: $(EXTERNALS:.c=.pd_linux)

# this might be a better way to do this:
# EXTERNALS    := ${patsubst %.c, %.o, ${wildcard *.c}} 
# all: ${EXTERNALS}

.SUFFIXES: .pd_linux

# Pentium III or better
#CPU_FLAGS = -march=pentium3
# PowerPC G3
#CPU_FLAGS = -mpowerpc-gpopt
# PowerPC G4
#CPU_FLAGS = -mpowerpc-gpopt -mcpu=7450 -maltivec
# PowerPC G5
#CPU_FLAGS = -mpowerpc64 -maltivec -falign-functions=32 -falign-labels=32 -falign-loops=32 -falign-jumps=32

OPTIM_FLAGS = -O3 -funroll-loops -fomit-frame-pointer $(CPU_FLAGS)

CFLAGS = -fPIC -DPD -DUNIX $(OPTIM_FLAGS)  \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch

INCLUDES = -I. -I.. -I../include

%.pd_linux: ../src/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o "$*.o" -c "../src/$*.c"
	gcc -Wl,-export_dynamic  -shared -o "$*.pd_linux" "$*.o" -lc -lm \
	`test -f $*.libs && cat $*.libs`
	chmod a-x "$*.pd_linux"
	$(STRIP) $*.pd_linux
	rm -f "$*.o" 

clean:
	-rm *.pd_linux *~ *.c
	-rm link.stamp

install-doc:
	@test -d $(prefix)/lib/pd/doc/5.reference || mkdir -p $(prefix)/lib/pd/doc/5.reference
	cd ../doc && make all
	cp -r ../doc/* $(prefix)/lib/pd/doc/5.reference

install: install-doc
	@test -d $(prefix)/lib/pd/extra || mkdir -p $(prefix)/lib/pd/extra
	install -m644 *.pd_linux $(prefix)/lib/pd/extra


deb:
	cd ../.. && dpkg-buildpackage -b
