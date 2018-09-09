CC=gcc
CFLAGS=-I./3rdparty/LuaJIT/src
LFLAGS=3rdparty/LuaJIT/src/libluajit.a

SRC=$(wildcard src/*.c) $(wildcard src/**/*.c)
EXE=bin/luashell.exe

.PHONY: all clean

all: luajit
	@rm -rf bin
	@mkdir bin
	@$(CC) -o $(EXE) $(SRC) $(CFLAGS) $(LFLAGS)
	@cp -r src/lua bin/lua

luajit:
	@$(MAKE) -C 3rdparty/LuaJIT BUILDMODE=static

clean:
	@rm -f bin/luashell.exe
	@$(MAKE) -C 3rdparty/LuaJIT clean
