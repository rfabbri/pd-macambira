current: all

.SUFFIXES: .pd_linux

INCLUDE = -I. -I/usr/local/src/pd/src

LDFLAGS = -export-dynamic -shared
LIB = -ldl -lm -lpthread

#select either the DBG and OPT compiler flags below:

CFLAGS = -DPD -DUNIX -W -Werror -Wno-unused \
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer \
        -DDL_OPEN

SYSTEM = $(shell uname -m)

# the sources

SRC = tab_copy.c \
	tab_reverse.c \
	tab_min_max.c \
	tab_min_index.c \
	tab_max_index.c \
	tab_find_peaks.c \
	tab_abs.c \
	tab_sqrt.c \
	tab_sum.c \
	tab_add.c \
	tab_sub.c \
	tab_mul.c \
	tab_div.c \
	tab_complex_mul.c \
	tab_complex_inv.c \
	tab_add_scalar.c \
	tab_mul_scalar.c \
	tab_const.c \
	tab_fft.c \
	tab_ifft.c \
	tab_rfft.c \
	tab_rifft.c \
	tab_cross_corr.c \
	tab_conv.c \
	tab_gt_scalar.c \
	tab_ge_scalar.c \
	tab_lt_scalar.c \
	tab_le_scalar.c \
	tab_ne_scalar.c \
	tab_eq_scalar.c \
	tab_gt.c \
	tab_ge.c \
	tab_lt.c \
	tab_le.c \
	tab_ne.c \
	tab_eq.c \
	tab_counter.c \
	iem_tab.c

TARGET = iem_tab.pd_linux


OBJ = $(SRC:.c=.o) 

#
#  ------------------ targets ------------------------------------
#

clean:
	rm $(TARGET)
	rm *.o

all: $(OBJ)
	@echo :: $(OBJ)
	ld $(LDFLAGS) -o $(TARGET) *.o $(LIB)
	strip --strip-unneeded $(TARGET)

$(OBJ) : %.o : %.c
	touch $*.c
	cc $(CFLAGS) $(INCLUDE) -c -o $*.o $*.c




