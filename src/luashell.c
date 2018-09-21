#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

#include "platforms.h"
#include "lualib_luashell.h"

static char* luashell_getline(char* buffer, int length)
{
    int c;
    
    while ((c = fgetc(stdin)) > 0 && c != '\n' && c != '\r')
    {
        *buffer++ = c;
        if (!length--)
        {
            return NULL;
        }
    }
    *buffer++ = 0;
    
    return buffer;
}

static char* luashell_faststrtrim(char* buffer)
{
    while (isspace(*buffer)) buffer++;
    
    char* str = buffer;
    while (*str) str++; str--;
    while (isspace(*str)) str--;
    str[1] = 0;
    return buffer;
}

static char** luashell_splitargs(char* input, int* outcount)
{
    static char** args;
    static int argscap;

    int count = 0;
    while (*input)
    {
        while (isspace(*input))
        {
            input++;
        }
        
        char* arg = input;
        while (*input && !isspace(*input))
        {
            input++;
        }

        if (input != arg)
        {
            if (*input)
            {
                *input++ = 0;
                while (isspace(*input))
                {
                    input++;
                }
            }
            
            if (count + 1 > argscap)
            {
                argscap = argscap > 0 ? argscap * 2 : 32;
                args = realloc(args, sizeof(char*) * argscap);
            }
            args[count++] = arg;
        }
        else
        {
            break;
        }
    }
    
    if (count + 1 > argscap)
    {
        argscap = argscap > 0 ? argscap * 2 : 32;
        args = realloc(args, sizeof(char*) * argscap);
    }
    args[count] = 0;

    if (outcount)
    {
        *outcount = count;
    }
    return args;
}

static void luashell_intro(void)
{
    printf(
        "LuaShell v1.0 by MaiHD\n"
        "\n");
}

static void luashell_prompt(void)
{
    static char cwd[1024];
    luashell_getcwd(cwd, sizeof(cwd) - 1);
    printf("%s> ", cwd);
}

typedef int (*luashell_command_func)(const char** args, int count);
struct luashell_command
{
    const char*           name;
    luashell_command_func func;
};

static int luashell_command_cd    (const char** args, int count);
static int luashell_command_set   (const char** args, int count);
static int luashell_command_get   (const char** args, int count);
static int luashell_command_help  (const char** args, int count);
static int luashell_command_exit  (const char** args, int count);
static int luashell_command_exec  (const char** args, int count);
static int luashell_command_clear (const char** args, int count);
static int luashell_command_mkdir (const char** args, int count);

static struct luashell_command commands[] = {
    { "cd"  ,  luashell_command_cd    },
    { "set" ,  luashell_command_set   },
    { "get" ,  luashell_command_get   },
    { "help",  luashell_command_help  },
    { "exit",  luashell_command_exit  },
    { "exec",  luashell_command_exec  },
    { "clear", luashell_command_clear },
    { "mkdir", luashell_command_mkdir },
};

static lua_State* lua_state;
static int luashell_openlua(void)
{
    lua_state = luaL_newstate();
    if (!lua_state)
    {
        return -1;
    }
    luaL_openlibs(lua_state);
    luaL_openluashell(lua_state);

    /* Save cwd */
    char cwd[1024];
    luashell_getcwd(cwd, sizeof(cwd));
    
    /* Get exe file name */
    char exepath[1024];
    luashell_exedir(exepath, sizeof(exepath));

    /* Initialize luashell runtime */
    char runtimepath[1024];
#ifdef LUASHELL_WINDOWS
    sprintf(runtimepath, "%s\\lua\\luashell.lua", exepath);
#else
    sprintf(runtimepath, "%s/lua/luashell.lua", exepath);
#endif
    
    if (luaL_loadfile(lua_state, runtimepath))
    {
        fprintf(stderr, "Error: failed to open %s\nPress any key to exit...", runtimepath);
        getchar();
        return -1;
    }
    if (lua_pcall(lua_state, 0, -1, 0) != 0)
    {
        fprintf(stderr, "Error: failed to initialize Lua runtime\nPress any key to exit...");
        getchar();
        return -1;
    }

    /* Return back to cwd */
    luashell_chdir(cwd);
    
    return 0;
}

static void luashell_closelua(void)
{
    lua_close(lua_state);
}

static int luashell_execute(const char** args, int count)
{
    assert(args);

    if (!args[0])
    {
        return 0;
    }
    else
    {
        int i;
        int n = sizeof(commands) / sizeof(commands[0]);
        for (i = 0; i < n; i++)
        {
            if (strcmp(args[0], commands[i].name) == 0)
            {
                return commands[i].func(args, count);
            }
        }

        int len = strlen(args[0]);
        if (strcmp(args[0] + len - 4, ".lua") == 0)
        {
            return luaL_loadfile(lua_state, args[0]) || lua_pcall(lua_state, 0, LUA_MULTRET, 0);
        }

        return luashell_launch(args, count);
    }
}

int luashell_command_cd(const char** args, int count)
{
    assert(args && strcmp("cd", args[0]) == 0);

    if (count > 2)
    {
        printf("Wrong syntax. Usage: cd <path>\n");
    }
    else if (count == 2)
    {
        int rc = luashell_chdir(args[1]);
        if (rc != 0)
        {
            perror("");
        }
    }
    else
    {
        char cwd[1024];
        luashell_getcwd(cwd, sizeof(cwd) - 1);
        printf("Current working directory: %s\n", cwd);
    }
    return 0;
}

int luashell_command_get(const char** args, int count)
{
    if (count != 2)
    {
        printf("Wrong syntax. Usage: get <variable>\n");
    }
    else
    {
        char var[1024];
        luashell_getenv(args[1], var, sizeof(var) - 1);
        printf("%s=%s", args[1], var);
    }
    return 0;
}

int luashell_command_set(const char** args, int count)
{
    if (count != 3)
    {
        printf("Wrong syntax. Usage: set <name> <value>\n");
    }
    else
    {
        luashell_setenv(args[1], args[2]);
    }
    return 0;
}

int luashell_command_help(const char** args, int count)
{
    printf(
        "LuaShell - Shell with Lua as scriptings\n\n"
        "cd       Change directory\n"
        "mkdir    Create directiory\n"
        "help     Show help documents\n"
        "exit     Exit the application\n"
        "exec     Exec a command\n"
        "clear    Clear screen\n"
        "get      Get environment variable\n"
        "set      Set environment variable\n"
    );
    
    return 0;
}

int luashell_command_exit(const char** args, int count)
{
    assert(args && strcmp("exit", args[0]) == 0);

    exit(EXIT_SUCCESS);
    return 0;
}

int luashell_command_exec(const char** args, int count)
{
    assert(args && strcmp("exec", args[0]) == 0);
    return luashell_launch(args + 1, count);
}

int luashell_command_clear(const char** args, int count)
{
    assert(args && strcmp("clear", args[0]) == 0);
    if (luashell_clrscr() == 0)
    {
        luashell_intro();
        return 0;
    }
    else
    {
        return -1;
    }
}

int luashell_command_mkdir(const char** args, int count)
{
    assert(args && strcmp("mkdir", args[0]) == 0);

    if (count != 2)
    {
        fprintf(stderr, "Wrong syntex. Usage: mkdir <path>\n");
    }
    else
    {
        luashell_mkdir(args[1]);
    }
    return 0;
}

static void _sighandler(int sig)
{
    switch (sig)
    {
    case SIGINT:
        /* NULL */
        break;
    }
}

int main(int argc, char* argv[])
{
    int count;
    int status = 0;
    char line[1024];
    char path[1024];

    /* skip Ctrl+C signal */
    signal(SIGINT, _sighandler);
    
    if (luashell_openlua() != 0)
    {
        return -1;
    }
    
    luashell_intro();
    do
    {
        luashell_prompt();
        luashell_getline(line, sizeof(line));

        char* input = luashell_faststrtrim(line);
        char** args = luashell_splitargs(input, &count);

        status = luashell_execute((const char**)args, count);

        printf("\n");
    } while (!status);

    luashell_closelua();
    return EXIT_SUCCESS;
}
