#include "lualib_luashell.h"

#include <stdlib.h>

static int _lua_exit(lua_State* state)
{
    exit(EXIT_SUCCESS);
    lua_pushinteger(state, 0);
    return 1;
}

static int _lua_exec(lua_State* state)
{
    static char** args;
    static int argscap;
    
    lua_pushinteger(state, 0);
    return 1;
}

void luaL_openluashell(lua_State* lua)
{
    lua_pushcfunction(lua, _lua_exit);
    lua_pushcfunction(lua, _lua_exec);
}
