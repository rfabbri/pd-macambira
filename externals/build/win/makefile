
# Adapt the PDPATH if your pd is not installed in the standard place

# path for compiling against an installed version of Pd
#PDPATH="$(PROGRAMFILES)/pd"
# path for compiling from CVS with the standard developer layout
PDPATH=../../../pd

EXTERNALS = $(shell ls ../src)

all: externals

externals: $(EXTERNALS:.c=.dll)

.SUFFIXES: .dll

# these are little macros for remapping things on Windows
WINDOWS_COMPAT_DEFINES = \
	-D'drand48()=((double)rand()/RAND_MAX)' \
	-D'srand48(n)=srand((n))' \
#	-D'bzero(p,n)=memset(p,0,n)' \
#	-D'PROT_READ=1' \
#	-D'MAP_PRIVATE=2' \
#	-D'O_NONBLOCK=1' \
#	-D'O_NDELAY=O_NONBLOCK'

DEFINES = -DPD -DNT $(WINDOWS_COMPAT_DEFINES)


# Generic x86 (tune for 686, since that's most common these days)
OPTIM_FLAGS = -O2 -march=i686 -mfpmath=sse -msse
# Pentium MMX
#OPTIM_FLAGS = -O3 -march=pentium-mmx -mmmx
# Pentium Pro
#OPTIM_FLAGS = -O3 -march=pentiumpro -mmmx
# Pentium II/Celeron
#OPTIM_FLAGS = -O3 -mfpmath=sse -mmmx -msse -march=pentium2
# Pentium III/Celeron2
#OPTIM_FLAGS = -O3 -mfpmath=sse -mmmx -msse -march=pentium3
# Pentium 4
#OPTIM_FLAGS = -O3 -mfpmath=sse -mmmx -msse -msse2 -march=pentium4

# AMD Athlon XP K7
#OPTIM_FLAGS = -O3 -march=athlon-xp -m3dnow -msse -mfpmath=sse

CFLAGS = -Wall -W -Wshadow -Wstrict-prototypes -Wno-unused -Wno-parentheses \
	 -Wno-switch $(OPTIM_FLAGS)

INCLUDE = -I. -I.. -I$(PDPATH)/src

DLLWRAP_FLAGS = --as=$(AS) --export-all --driver-name $(CC) -s 

%.dll: ../src/%.c
	$(CC) -mms-bitfields $(CFLAGS) $(DEFINES) $(INCLUDE) \
		 -o "$*.o" -c "../src/$*.c"
	gcc -shared -o "$*.dll" "$*.o" $(PDPATH)/bin/pd.dll \
		`test -f $*.libs && cat $*.libs` \
		`test -f ../src/$*.libs && cat ../src/$*.libs` 

clean:
	-rm *.a *.def *.dll *.o *.obj

install-doc:
	@test -d $(PDPATH)/doc/5.reference || mkdir -p $(PDPATH)/doc/5.reference
	cd ../doc && make all
	cp -r ../doc/* $(PDPATH)/doc/5.reference

install: install-doc
	@test -d $(PDPATH)/extra || mkdir -p $(PDPATH)/extra
	install *.dll $(PDPATH)/extra
