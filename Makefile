CC=gcc
CFLAGS=-I./3rdparty/LuaJIT/src
LFLAGS=3rdparty/LuaJIT/src/libluajit.a

SRC=src/luashell.c src/platforms.unix.c src/platforms.windows.c
EXE=bin/luashell.exe

.PHONY: all clean

all: $(luajit)
	@rm -rf bin
	@mkdir bin
	@$(CC) -o $(EXE) $(SRC) $(CFLAGS) $(LFLAGS)

luajit:
	@$(MAKE) -C 3rdparty/LuaJIT BUILDMODE=static

clean:
	@rm -f bin/luashell.exe
	@$(MAKE) -C 3rdparty/LuaJIT clean
