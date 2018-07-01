#pragma once
#include "socket.h"

#ifdef WIN32
#define SPRINTF(a, b, c, ...) sprintf_s(a, b, c, __VA_ARGS__)
#define FREAD(a, b, c, d, e) fread_s(a, b, c, d, e)
#define STRNCPY(a, b, c, d) strncpy_s(a, b, c, d)
#define STRNCAT(a, b, c, d) strncat_s(a, b, c, d)
#define CLOSESOCKET(a) closesocket(a)
#else
#define SPRINTF(a, b, c, ...) sprintf(a, c, __VA_ARGS__)
#define FREAD(a, b, c, d, e) fread(a, c, d, e)
#define STRNCPY(a, b, c, d) strncpy(a, c, d+1)			// +1 for the terminating null character
#define STRNCAT(a, b, c, d) strncat(a, c, d)
#define CLOSESOCKET(a) close(a)
#define SD_BOTH (SHUT_RDWR)
#endif // WIN32

extern int fopenPortable(FILE **fp, const char *filename, const char *mode);