#ifndef __LUASHELL_PLATFORMS_H__
#define __LUASHELL_PLATFORMS_H__

#if defined(__CYGWIN__) || defined(_MSC_VER)
#define LUASHELL_WINDOWS
#elif defined(__unix__)
#define LUASHELL_UNIX
#else
#error "Compile failed: unknown or unsupported target platform"
#endif

int luashell_chdir(const char* path);
int luashell_getcwd(char* buffer, int length);
int luashell_launch(const char** args, int count);

int luashell_fileexists(const char* path);
int luashell_isdirectory(const char* path);

#endif /* __LUASHELL_PLATFORMS_H__ */
