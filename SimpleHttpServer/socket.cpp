#include <stdio.h>
#include "error.h"

#ifdef WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#endif

#ifdef WIN32
int InitWinSock() {
	WSADATA data = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		fprintf(stderr, "WSAStartup failed!");
		return ERROR_STARTUP;
	}

	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2) {
		fprintf(stderr, "No usable version of WinSock.dll available");
		WSACleanup();
		return ERROR_STARTUP;
	}

	return 0;
}
#endif

int CloseSocket(unsigned int sock) {
	int ret;

	ret = closesocket(sock);
	if (ret == SOCKET_ERROR) {
		fprintf(stderr, "closesocket failed!");
		return ERROR_CLOSESOCKET;
	}

	return 0;
}

int ShutdownSocket(unsigned int sock) {
	int ret;

	ret = shutdown(sock, SD_BOTH);
	if (ret == SOCKET_ERROR) {
		fprintf(stderr, "shutdown failed!");
		return ERROR_SHUTDOWN;
	}

	return 0;
}

unsigned int ListenForHttpConnection(int port) {
	unsigned int sock;
	SOCKADDR_IN sockaddrIn = { 0 };
	int ret;

	// create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		fprintf(stderr, "socket failed!");
		return ERROR_SOCKET;
	}

	sockaddrIn.sin_family = AF_INET;
	inet_pton(AF_INET, INADDR_ANY, &sockaddrIn.sin_addr.S_un.S_addr);
	sockaddrIn.sin_port = htons(port);

	// bind to port
	ret = bind(sock, (SOCKADDR *)&sockaddrIn, sizeof(sockaddrIn));
	if (ret == SOCKET_ERROR) {
		fprintf(stderr, "bind failed!");
		CloseSocket(sock);
		return ERROR_BIND;
	}

	// listen on port
	ret = listen(sock, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		fprintf(stderr, "listen failed!");
		CloseSocket(sock);
		return ERROR_LISTEN;
	}

	return sock;
}

unsigned int AcceptConnection(unsigned int sock, SOCKADDR_IN *clientSockaddrIn, int sizeSockaddrIn) {
	unsigned int clientSock;

	// accept incoming connection
	clientSock = accept(sock, (SOCKADDR *)clientSockaddrIn, &sizeSockaddrIn);
	if (clientSock == SOCKET_ERROR) {
		fprintf(stderr, "accept failed!");
		return ERROR_ACCEPT;
	}

	return clientSock;
}

int ReceiveData(unsigned int clientSock, char *buf, int bufsize) {
	return recv(clientSock, buf, bufsize, 0);
}

int SendData(unsigned int clientSock, const char *buf, int bufsize) {
	return send(clientSock, buf, bufsize, 0);
}