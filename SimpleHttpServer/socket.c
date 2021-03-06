#include <stdio.h>
#include "config.h"
#include "error.h"
#include "platform.h"
#include "socket.h"

#ifdef _WIN32
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

	ret = CLOSESOCKET(sock);
	if (ret < 0) {
		fprintf(stderr, "closesocket failed!");
		return ERROR_CLOSESOCKET;
	}

	return 0;
}

int ShutdownSocket(unsigned int sock) {
	int ret;

	ret = shutdown(sock, SD_BOTH);
	if (ret < 0) {
		fprintf(stderr, "shutdown failed!");
		return ERROR_SHUTDOWN;
	}

	return 0;
}

unsigned int ListenForHttpConnection(unsigned int port) {
	unsigned int sock;
	struct sockaddr_in sockaddrIn = { 0 };
	int ret;

	// create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((signed int)sock == -1) {
		fprintf(stderr, "socket failed!");
		return (unsigned int)~0;
	}

	sockaddrIn.sin_family = AF_INET;
	sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
	sockaddrIn.sin_port = htons(port);

	// bind to port
	ret = bind(sock, (struct sockaddr *)&sockaddrIn, sizeof(sockaddrIn));
	if (ret < 0) {
		fprintf(stderr, "bind failed!");
		CloseSocket(sock);
		return (unsigned int)~0;
	}

	// listen on port
	ret = listen(sock, SOMAXCONN);
	if (ret < 0) {
		fprintf(stderr, "listen failed!");
		CloseSocket(sock);
		return (unsigned int)~0;
	}

	return sock;
}

unsigned int AcceptConnection(unsigned int sock, struct sockaddr_in *clientSockaddrIn, int sizeSockaddrIn) {
	unsigned int clientSock;

	// accept incoming connection
	clientSock = accept(sock, (struct sockaddr *)clientSockaddrIn, &sizeSockaddrIn);
	if ((signed int)clientSock == -1) {
		fprintf(stderr, "accept failed!");
		return (unsigned int)~0;
	}

	return clientSock;
}

int ReceiveData(unsigned int clientSock, char *buf, int bufsize) {
	return recv(clientSock, buf, bufsize, 0);
}

int SendData(unsigned int clientSock, const char *buf, int bufsize) {
	return send(clientSock, buf, bufsize, 0);
}

unsigned int OpenCGIClientSock() {
	unsigned int sock;
	struct sockaddr_in sockaddrIn = { 0 };
	int ret;

	// create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if ((signed int)sock == -1) {
		fprintf(stderr, "socket failed!");
		return (unsigned int)~0;
	}

	sockaddrIn.sin_family = AF_INET;
	sockaddrIn.sin_addr.s_addr = inet_addr(config.fcgiHost);
	sockaddrIn.sin_port = htons(config.fcgiPort);
	
	ret = connect(sock, (struct sockaddr *)&sockaddrIn, sizeof(sockaddrIn));
	if (ret < 0) {
		fprintf(stderr, "connect failed!");
		return (unsigned int)~0;
	}

	return sock;
}