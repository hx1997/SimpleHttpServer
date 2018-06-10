#include <stdio.h>

#ifdef WIN32
	#include <WinSock2.h>
	#include <Ws2tcpip.h>

	#pragma comment(lib, "Ws2_32.lib")
#else
	#include <sys/socket.h>
#endif

// default www root path
#define WWW_ROOT_PATH		"./www"
#define WWW_INDEX_FILE		"/index.html"
#define WWW_INDEX_PATH		(WWW_ROOT_PATH WWW_INDEX_FILE)
// default port number
#define PORT_NO				(8080)

// http response codes
#define HTTP_OK				"200 OK"

// error codes
#define ERROR_STARTUP       (-1)
#define ERROR_SOCKET        (-2)
#define ERROR_BIND          (-3)
#define ERROR_LISTEN		(-4)
#define ERROR_ACCEPT		(-5)
#define ERROR_SHUTDOWN      (-6)
#define ERROR_CLOSESOCKET   (-7)
#define ERROR_FOPEN			(-8)

#define BUFSIZE 512

#ifdef WIN32
int InitWinSock() {
	WSADATA data = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
		perror("WSAStartup failed!");
		return ERROR_STARTUP;
	}

	if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2) {
		perror("No usable version of WinSock.dll available");
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
		perror("closesocket failed!");
		return ERROR_CLOSESOCKET;
	}

	return 0;
}

int ShutdownSocket(unsigned int sock) {
	int ret;

	ret = shutdown(sock, SD_BOTH);
	if (ret == SOCKET_ERROR) {
		perror("shutdown failed!");
		return ERROR_SHUTDOWN;
	}

	return 0;
}

unsigned int ListenForHttpConnection() {
	unsigned int sock;
	SOCKADDR_IN sockaddrIn = { 0 };
	int ret;

	// create socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		perror("socket failed!");
		return ERROR_SOCKET;
	}

	sockaddrIn.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &sockaddrIn.sin_addr.S_un.S_addr);
	sockaddrIn.sin_port = htons(PORT_NO);

	// bind to localhost:PORT_NO
	ret = bind(sock, (SOCKADDR *)&sockaddrIn, sizeof(sockaddrIn));
	if (ret == SOCKET_ERROR) {
		perror("bind failed!");
		CloseSocket(sock);
		return ERROR_BIND;
	}

	// listen on PORT_NO
	ret = listen(sock, SOMAXCONN);
	if (ret == SOCKET_ERROR) {
		perror("listen failed!");
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
		perror("accept failed!");
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

int SendHttpHeader(unsigned int clientSock, const char *responseCode, const char *contentType) {
	int datalen;
	char buf[BUFSIZE];

	datalen = sprintf_s(buf, "HTTP/1.1 %s\r\nServer : SimpleHttpServer\r\nConnection : close\r\nContent - Type : %s\r\n\r\n", \
		responseCode, contentType);

	return SendData(clientSock, buf, datalen);
}

int SendTextFile(unsigned int clientSock, const char *file) {
	int ret;
	FILE *fp;
	char buf[BUFSIZE];

	if (fopen_s(&fp, file, "r") != 0) {
		return ERROR_FOPEN;
	}

	while (!feof(fp)) {
		int datalen = fread_s(buf, BUFSIZE, 1, BUFSIZE, fp);
		ret = SendData(clientSock, buf, datalen);
		if (ret < 0) {
			fclose(fp);
			return ret;
		}
	}

	fclose(fp);
	return 0;
}

int main(int argc, char **argv) {
	int ret;

	// init winsock
#ifdef WIN32
	ret = InitWinSock();
	if (ret < 0) {
		return ret;
	}
#endif

	printf("SimpleHttpServer started.\n");
	
	unsigned int sock = ListenForHttpConnection();
	if (sock < 0) {
		return sock;
	}
	printf("Listening on port %d for incoming HTTP connections.\n", PORT_NO);

	while (1) {
		SOCKADDR_IN clientSockaddrIn = { 0 };
		// wait for connection
		unsigned int clientSock = AcceptConnection(sock, &clientSockaddrIn, sizeof(clientSockaddrIn));
		if (clientSock < 0) {
			CloseSocket(sock);
			return clientSock;
		}

		printf("\nIncoming connection\n");
		printf("Client socket = %s:%d\n", inet_ntoa(clientSockaddrIn.sin_addr), clientSockaddrIn.sin_port);
		
		char recvBuf[BUFSIZE] = { 0 };

		ret = ReceiveData(clientSock, recvBuf, BUFSIZE);
		if (ret > 0) {
			printf("Received data:\n%s\n", recvBuf);

			char *getPathStart = recvBuf;
			char *getPathEnd = NULL;
			while (*getPathStart != ' ') getPathStart++;
			while (*getPathStart == ' ') getPathStart++;
			getPathEnd = getPathStart;
			while (*getPathEnd != ' ') getPathEnd++;
			*getPathEnd = '\0';

			char filePath[BUFSIZE];

			if (getPathEnd - getPathStart == 1 && *getPathStart == '/') {
				sprintf_s(filePath, WWW_INDEX_PATH);
			}
			else {
				sprintf_s(filePath, WWW_ROOT_PATH "%s", getPathStart);
			}

			SendHttpHeader(clientSock, HTTP_OK, "text / html");
			SendTextFile(clientSock, filePath);
		}

		ShutdownSocket(clientSock);
		CloseSocket(clientSock);
	}

	CloseSocket(sock);

	// cleanup winsock
#ifdef WIN32
	WSACleanup();
#endif

	return 0;
}