all: pd_linux

SUFFIX=pd_linux
#SUFFIX=pd_darwin

$(SUFFIX): tracker.$(SUFFIX)
tracker.$(SUFFIX): tracker.c tracker.h tracker.tk2c

pd_darwin: tracker.pd_darwin
tracker.pd_: tracker.c tracker.h tracker.tk2c

STRIP = strip
CFLAGS = -DPD -O2  \
    -Wall -W -Wno-shadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch
INCLUDE = -I/usr/include -I../pd/src
LDFLAGS = 

ifeq ($(SUFFIX),pd_linux)
	CFLAGS += -DUNIX -funroll-loops -fomit-frame-pointer
	LDFLAGS += -export_dynamic -shared -lc -lm
endif
ifeq ($(SUFFIX),pd_darwin)
	LDFLAGS += -bundle -undefined suppress -flat_namespace
	STRIP = false
endif

.SUFFIXES: .$(SUFFIX) .tk .tk2c

.tk.tk2c:
	./tk2c.bash < $*.tk > $*.tk2c
.c.$(SUFFIX):
	$(CC) $(CFLAGS) $(INCLUDE) -o $*.o -c $*.c
	$(LD) $(LDFLAGS) -o $*.pd_linux $*.o
	$(STRIP) --strip-unneeded $*.pd_linux
clean:
	$(RM) -f *.o *.pd_* so_locations *.tk2c
