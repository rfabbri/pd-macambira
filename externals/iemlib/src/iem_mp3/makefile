current: all

.SUFFIXES: .pd_linux

INCLUDE = -I. -I/usr/local/src/pd-0.37-4/src

LDFLAGS = -export-dynamic -shared
LIB = -ldl -lm -lpthread

#select either the DBG and OPT compiler flags below:

CFLAGS = -DPD -DUNIX -W -Werror -Wno-unused \
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer \
        -DDL_OPEN -fPIC

SYSTEM = $(shell uname -m)

# the sources

SRC = sigmp3play.c \
	iem_mp3.c

TARGET = iem_mp3.pd_linux


OBJ = $(SRC:.c=.o) 

#
#  ------------------ targets ------------------------------------
#

clean:
	-rm ../../lib/$(TARGET) $(TARGET)
	-rm *.o

all: $(OBJ)

	@echo :: $(OBJ)

	ld $(LDFLAGS) -o $(TARGET) *.o $(LIB)
	strip --strip-unneeded $(TARGET)
	cp $(TARGET) ../../lib

$(OBJ) : %.o : %.c
	touch $*.c
	cc $(CFLAGS) $(INCLUDE) -c -o $*.o $*.c



