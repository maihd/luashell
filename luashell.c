//
// C runtime
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

//
// Lua runtime
//
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

#include "platforms.h"

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

typedef int (*luashell_command_func)(const char** args, int count);
struct luashell_command
{
    const char*           name;
    luashell_command_func func;
};

static int luashell_command_cd(const char** args, int count);
//static int luashell_command_ls(const char** args);
static int luashell_command_exit(const char** args, int count);

static struct luashell_command commands[] = {
    { "cd", luashell_command_cd },
    { "exit", luashell_command_exit },
};

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
    return 0;
}

int luashell_command_exit(const char** args, int count)
{
    assert(args && strcmp("exit", args[0]) == 0);

    exit(EXIT_SUCCESS);
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
    
    lua_State* lua;
    lua = luaL_newstate();
    if (!lua)
    {
        return -1;
    }
    luaL_openlibs(lua);
    
    do
    {
        luashell_getcwd(path, sizeof(path));
        printf("%s> ", path);
        luashell_getline(line, sizeof(line));

        char* input = luashell_faststrtrim(line);
        char** args = luashell_splitargs(input, &count);

        status = luashell_execute((const char**)args, count);
    } while (!status);

    lua_close(lua);
    return EXIT_SUCCESS;
}
