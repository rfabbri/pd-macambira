current: all

.SUFFIXES: .pd_linux

INCLUDE = -I. -I/usr/local/src/pd/src

LDFLAGS = -export-dynamic -shared
LIB = -ldl -lm -lpthread

#select either the DBG and OPT compiler flags below:

CFLAGS = -DPD -DUNIX -Wall -W -Werror -Wno-unused \
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer \
        -DDL_OPEN

SYSTEM = $(shell uname -m)

# the sources

SRC = add2_comma.c \
	bpe.c \
	dollarg.c \
	exp_inc.c \
	float24.c \
	iem_anything_kernel.c \
	iem_append_kernel.c \
	iem_i_route.c \
	iem_pbank_csv.c \
	iem_prepend_kernel.c \
	iem_receive_kernel.c \
	iem_route.c \
	iem_sel_any.c \
	iem_send_kernel.c \
	init.c \
	list2send.c \
	mergefilename.c \
	modulo_counter.c \
	post_netreceive.c \
	pre_inlet.c \
	prepend_ascii.c \
	round_zero.c \
	sigfade.c \
	sigiem_blocksize.c \
	sigiem_samplerate.c \
	sigLFO_noise.c \
	sigm2f.c \
	speedlim.c \
	splitfilename.c \
	stripfilename.c \
	toggle_mess.c \
	transf_fader.c \
	unsymbol.c \
	wrap.c \
	iemlib2.c

TARGET = iemlib2.pd_linux


OBJ = $(SRC:.c=.o) 

#
#  ------------------ targets ------------------------------------
#

clean:
	rm ../../lib/$(TARGET)
	rm *.o

all: $(OBJ)
	@echo :: $(OBJ)
	ld $(LDFLAGS) -o $(TARGET) *.o $(LIB)
	strip --strip-unneeded $(TARGET)
	mv $(TARGET) ../../lib
	rm *.o

$(OBJ) : %.o : %.c
	touch $*.c
	cc $(CFLAGS) $(INCLUDE) -c -o $*.o $*.c




