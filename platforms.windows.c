#include "platforms.h"

#ifdef LUASHELL_WINDOWS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

int luashell_getcwd(char* buffer, int length)
{
    return (int)GetCurrentDirectoryA(length, buffer);
}

int luashell_chdir(const char* path)
{
    return !SetCurrentDirectoryA(path);
}

int luashell_launch(const char** args, int count)
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&pi, sizeof(pi));
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    
    char cmdline[1024] = "";
    for (int i = 0; i < count; i++)
    {
        strcat(cmdline, args[i]);
        strcat(cmdline, " ");
    }
    
    if (!CreateProcessA(
            NULL,
            cmdline,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi))
    {
        printf("Command not found\n");
        return 0;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    
    return 0;
}

#endif
