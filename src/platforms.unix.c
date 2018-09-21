#include "platforms.h"

#ifdef LUASHELL_UNIX

#include <unistd.h>
#include <sys/wait.h>

int luashell_getcwd(char* buffer, int length)
{
    return getcwd(buffer, length);
}

int luashell_chdir(const char* path)
{
    return chdir(path);
}

int luashell_mkdir(const char* path)
{
    return mkdir(path);
}

int luashell_launch(const char** args, int count)
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

int luashell_clrscr(void)
{
    printf("\033[H\033[J");
    return 0;
}

int luashell_fileexists(const char* path)
{
    FILE* file = fopen(path, "r");
    if (file)
    {
        fclose(file);
        return 1;
    }
    else
    {
        return 0;
    }
}

int luashell_isdirectory(const char* path)
{
    DIR* dir = opendir(path);
    if (dir)
    {
        closedir(dir);
        return 1;
    }
    else
    {
        return 0;
    }
}

int luashell_setenv(const char* name, const char* value)
{
    return setenv(name, value, 1);
}

int luashell_getenv(const char* name, char* buffer, int length)
{
    char* value = getenv(name);
    if (value)
    {
        strncpy(buffer, value, length);
        return 0;
    }
    else
    {
        buffer[0] = 0;
        return -1;
    }
}

int luashell_homepath(char* buffer, int length)
{
    return luashell_getenv("HOME", buffer, length);
}
#endif
