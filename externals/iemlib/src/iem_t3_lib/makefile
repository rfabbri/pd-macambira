current: all

.SUFFIXES: .pd_linux
include ../../Make.include

INCLUDE = -I. -I$(PDSOURCE)

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




