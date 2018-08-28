//
// C runtime
//

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

//
// Lua runtime
//
#include <lua.h>
#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

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

static char** luashell_splitargs(char* input)
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
            *input++ = 0;
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

    return args;
}

typedef int (*luashell_command_func)(const char** args);
struct luashell_command
{
    const char*           name;
    luashell_command_func func;
};

static int luashell_command_cd(const char** args);
//static int luashell_command_ls(const char** args);
static int luashell_command_exit(const char** args);

static struct luashell_command commands[] = {
    { "cd", luashell_command_cd },
    { "exit", luashell_command_exit },
};

static int luashell_launch(const char** args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0)
    {
        if (execvp(args[0], (void*)args) == -1)
        {
            perror("luashell");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("luashell");
    }
    else
    {
        do
        {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 0;
}

static int luashell_execute(const char** args)
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
                return commands[i].func(args);
            }
        }

        return luashell_launch(args);
    }
}

int luashell_command_cd(const char** args)
{
    assert(args && strcmp("cd", args[0]) == 0);

    int argscount = 0;
    while (args[argscount])
    {
        argscount++;
    }

    if (argscount > 2)
    {
        printf("Wrong arguments number\nUsage: cd <path>\n");
        return 0;
    }
    else if (argscount < 2)
    {
        return 0;
    }
    else
    {
        return chdir(args[1]);
    }
}

int luashell_command_exit(const char** args)
{
    assert(args && strcmp("exit", args[0]) == 0);

    exit(EXIT_SUCCESS);
    return 0;
}

int main(int argc, char* argv[])
{
    int status = 0;
    char line[1024];
    char path[1024];

    lua_State* lua;
    lua = luaL_newstate();
    if (!lua)
    {
        return -1;
    }
    luaL_openlibs(lua);
    
    do
    {
        getcwd(path, sizeof(path));
        printf("%s> ", path);
        luashell_getline(line, sizeof(line));

        char* input = luashell_faststrtrim(line);
        char** args = luashell_splitargs(input);

        status = luashell_execute((const char**)args);
    } while (!status);

    lua_close(lua);
    return EXIT_SUCCESS;
}
