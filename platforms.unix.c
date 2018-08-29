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

#endif
