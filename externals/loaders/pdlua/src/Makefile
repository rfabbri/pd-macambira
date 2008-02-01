CFLAGS = -ansi -pedantic -Wall -fPIC
# for Debian Etch
CFLAGS += -I$(HOME)/include `pkg-config --cflags lua5.1`
# for pure:dyne, probably harmless on other OS
CFLAGS += -I/opt/pure/include -I/opt/lua/include -L/opt/lua/lib

all: lua.pd_linux

clean:
	rm -f lua.pd_linux

lua.pd_linux: lua.c
	gcc $(CFLAGS) -shared -o lua.pd_linux lua.c `pkg-config --libs lua5.1`

# FIXME: check this works, see about fat/universal binaries
lua.pd_darwin: lua.c
	gcc $(CFLAGS) -bundle -undefined suppress -flat_namespace -o lua.pd_darwin lua.c -llua

# FIXME: Windows?
