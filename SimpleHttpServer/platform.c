#include <stdio.h>

// @return
// 0 - success
// 1 - error
int fopenPortable(FILE **fp, const char *filename, const char *mode) {
#ifdef WIN32
	return fopen_s(fp, filename, mode);
#else
	*fp = fopen(filename, mode);
	if (*fp == NULL) {
		return 1;
	}
	return 0;
#endif // WIN32
}