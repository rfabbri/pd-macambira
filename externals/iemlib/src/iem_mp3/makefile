current: all

.SUFFIXES: .pd_linux
include ../../Make.include

INCLUDE = -I. -I$(PDSOURCE)

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



