current: all

.SUFFIXES: .pd_linux

include ../../Make.include

INCLUDE = -I. -I$(PDSOURCE)

# the sources
SRC = $(sort $(filter %.c, $(wildcard *.c)))

TARGET = iemlib1.pd_linux


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
	-cp $(TARGET) ../../lib

$(OBJ) : %.o : %.c
	touch $*.c
	cc $(CFLAGS) $(INCLUDE) -c -o $*.o $*.c




