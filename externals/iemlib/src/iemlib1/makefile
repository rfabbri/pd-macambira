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

SRC = biquad_freq_resp.c \
	db2v.c \
	f2note.c \
	forpp.c \
	gate.c \
	sigfilter.c \
	sigFIR.c \
	sighml_shelf.c \
	sigiem_cot4.c \
	sigiem_delay.c \
	sigiem_pow4.c \
	sigiem_sqrt4.c \
	siglp1_t.c \
	sigmov_avrg_kern.c \
	sigpara_bp2.c \
	sigpeakenv.c \
	sigprvu.c \
	sigpvu.c \
	sigrvu.c \
	sigsin_phase.c \
	sigvcf_filter.c \
	soundfile_info.c \
	split.c \
	v2db.c \
	iemlib1.c

TARGET = iemlib1.pd_linux


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




