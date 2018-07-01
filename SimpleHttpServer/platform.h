#pragma once
#ifdef WIN32
#define SPRINTF(a, b, c, ...) sprintf_s(a, b, c, __VA_ARGS__)
#define FOPEN(a, b, c) fopen_s(a, b, c)
#else
#define SPRINTF(a, b, c, ...) sprintf(a, c, __VA_ARGS__)
#endif // WIN32
