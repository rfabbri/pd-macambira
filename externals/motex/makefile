
#make file for motex Pd externals - Iain Mott

current:
	echo make pd_linux


# ----------------------- LINUX i386 -----------------------


pd_linux: pan~.pd_linux pansig~.pd_linux polygate~.pd_linux shuffle.pd_linux system.pd_linux ln~.pd_linux rec2pol~.pd_linux pol2rec~.pd_linux getenv.pd_linux


.SUFFIXES: .pd_linux

# -------------- postgres ----------------------

#SRCDIR= /usr/src/pgsql/postgresql-7.0.3/src
#EXTERNALDIR= /home/iain/puredata/summon2/externals/
TARGDIR= ./


#include Makefile.global #needed for postgresql stuff

CFLAGS+= -I$(LIBPQDIR)

LDFLAGS+= -L$(LIBPQDIR) -lpq

#-----------------------------------------------
EXSRCDIR= .

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch

# LINUXINCLUDE =  -I/home/iain/puredata/pd029/src
LINUXINCLUDE =  -I/home/iain/puredata/pd032/src
#LINUXINCLUDE+= -I/usr/include/g++-2
#-------------
#LINUXCFLAG+=CFLAGS

#LINUXINCLUDE+= -I/usr/local/pgsql/include
#--------------


.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $(EXSRCDIR)$*.o -c $*.c 
	ld -export_dynamic  -shared -o $(TARGDIR)$*.pd_linux $(EXSRCDIR)$*.o -lc   -lm $(LDFLAGS)
	strip --strip-unneeded $(TARGDIR)$*.pd_linux
	rm $(EXSRCDIR)$*.o 
#	cp $(TARGDIR)$*.pd_linux $(EXTERNALDIR)



#clean:
#	rm -f foo1.pd_linux burstan.pd_linux burstan~.pd_linux 




























