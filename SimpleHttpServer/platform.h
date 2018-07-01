#pragma once
#ifdef WIN32
#define CLOSESOCKET(a) closesocket(a)
#else
#define CLOSESOCKET(a) close(a)
#endif // WIN32
