CC=gcc
CFLAGS=-I./LuaJIT/src
LFLAGS=LuaJIT/src/libluajit.a

SRC=luashell.c platforms.unix.c platforms.windows.c
EXE=luashell.exe

.PHONY: all clean

all:
	@$(MAKE) -C LuaJIT BUILDMODE=static
	@$(CC) -o $(EXE) $(SRC) $(CFLAGS) $(LFLAGS)

clean:
	@rm -f luashell.exe
	@$(MAKE) -C LuaJIT clean
