PD_SRC=../../../pd/src
FFMPEG_HEADERS=-I/usr/include/ffmpeg

NAME=videogrid
CSYM=videogrid

current: pd_linux

# ----------------------- LINUX -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -fPIC -DPD -DUNIX -DICECAST -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wno-shadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch


LINUXINCLUDE =  -I$(PD_SRC) -I/usr/include/

.c.pd_linux:
	g++  $(FFMPEG_HEADERS) -fPIC -c -O -o videogrid.o videogrid.cpp
# 	./tk2c.bash < $*.tk > $*.tk2c
	g++ $(FFMPEG_HEADERS) -export_dynamic  -shared -o videogrid.pd_linux videogrid.o -lavformat -lavcodec -lavutil -lavcodec -lavutil -lquicktime -lgsm -lz -lm
	rm -f $*.o 

# ----------------------------------------------------------

install:
	cp *-help.pd ../../../doc/5.reference

clean:
	rm -f *.o *.pd_* so_locations

