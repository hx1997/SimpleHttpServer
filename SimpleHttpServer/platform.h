#pragma once
#include "socket.h"

#ifdef WIN32
#define CLOSESOCKET(a) closesocket(a)
#else
#define CLOSESOCKET(a) close(a)
#define SD_BOTH (SHUT_RDWR)
#endif // WIN32
