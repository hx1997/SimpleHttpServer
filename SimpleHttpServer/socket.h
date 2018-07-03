#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

extern int InitWinSock();
extern unsigned int ListenForHttpConnection(unsigned int port);
extern unsigned int AcceptConnection(unsigned int sock, struct sockaddr_in *clientSockaddrIn, int sizeSockaddrIn);
extern int ReceiveData(unsigned int clientSock, char *buf, int bufsize);
extern int SendData(unsigned int clientSock, const char *buf, int bufsize);
extern int CloseSocket(unsigned int sock);
extern int ShutdownSocket(unsigned int sock);
extern unsigned int OpenCGIClientSock();