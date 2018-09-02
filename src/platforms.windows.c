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

int luashell_mkdir(const char* path)
{
    return !CreateDirectoryA(path, NULL);
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

int luashell_cls(HANDLE hConsole)
{
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return -1;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hConsole,
                                    ' ',
                                    dwConSize,
                                    coordScreen,
                                    &cCharsWritten))
    {
        return -1;
    }

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        return -1;
    }

    if (!FillConsoleOutputAttribute(hConsole,
                                    csbi.wAttributes,
                                    dwConSize,
                                    coordScreen,
                                    &cCharsWritten))
    {
        return -1;
    }

    SetConsoleCursorPosition(hConsole, coordScreen);
    return 0;
}

int luashell_clrscr(void)
{
    return luashell_cls(GetStdHandle(STD_OUTPUT_HANDLE)) + luashell_cls(GetStdHandle(STD_ERROR_HANDLE));
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
    return GetFileAttributesA(path) & FILE_ATTRIBUTE_DIRECTORY;
}

#endif
