#pragma once
#ifdef WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#endif

extern int InitWinSock();
extern unsigned int ListenForHttpConnection(int port);
extern unsigned int AcceptConnection(unsigned int sock, SOCKADDR_IN *clientSockaddrIn, int sizeSockaddrIn);
extern int ReceiveData(unsigned int clientSock, char *buf, int bufsize);
extern int SendData(unsigned int clientSock, const char *buf, int bufsize);
extern int CloseSocket(unsigned int sock);
extern int ShutdownSocket(unsigned int sock);