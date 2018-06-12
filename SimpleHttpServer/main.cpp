#include <stdio.h>
#include "socket.h"
#include "error.h"

// default www root path
#define WWW_ROOT_PATH		"./www"
#define WWW_INDEX_FILE		"/index.html"
#define WWW_INDEX_PATH		(WWW_ROOT_PATH WWW_INDEX_FILE)
// default port number
#define PORT_NO				(8080)

// http response codes
#define HTTP_OK				"200 OK"
#define HTTP_NOT_FOUND		"404 Not Found"


#define BUFSIZE 512

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
	
	unsigned int sock = ListenForHttpConnection(PORT_NO);
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
			if (SendTextFile(clientSock, filePath) < 0) {
				SendData(clientSock, "<html><head><title>404 Not Found</title></head><body bgcolor = \"white\">\
					<center><h1>404 Not Found</h1></center><hr><center>SimpleHttpServer</center></body></html>", 159);
			}
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