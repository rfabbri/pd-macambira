current: all

.SUFFIXES: .pd_linux

INCLUDE = -I. -I/usr/local/src/pd-0.37-4/src

LDFLAGS = -export-dynamic -shared
LIB = -ldl -lm -lpthread

#select either the DBG and OPT compiler flags below:

CFLAGS = -DPD -DUNIX -W -Werror -Wno-unused \
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer \
        -DDL_OPEN

SYSTEM = $(shell uname -m)

# the sources

SRC = sigt3_line.c \
	sigt3_sig.c \
	t3_bpe.c \
	t3_delay.c \
	t3_metro.c \
	t3_timer.c \
	iem_t3_lib.c

TARGET = iem_t3_lib.pd_linux


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




