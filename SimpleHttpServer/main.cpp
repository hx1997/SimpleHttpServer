#include <stdio.h>
#include "socket.h"
#include "parse.h"
#include "http.h"
#include "error.h"

// default www root path
#define WWW_ROOT_PATH		"./www"
#define WWW_INDEX_FILE		"/index.html"
#define WWW_INDEX_PATH		(WWW_ROOT_PATH WWW_INDEX_FILE)
// default port number
#define PORT_NO				(8080)

#define BUFSIZE 512


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

	while (true) {
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
			HttpRequestMessage req = { 0 };
			ret = ParseHttpRequestMessage(recvBuf, &req);
			if (ret < 0) {
				if (ret == ERROR_NOT_IMPLEMENTED) {
					SendHttpHeader(clientSock, HTTP_NOT_IMPLEMENTED, "text/html");
					SendData(clientSock, NOT_IMPLEMENTED_HTML, strlen(NOT_IMPLEMENTED_HTML));
					goto finalize;
				}
			}

			char filePath[BUFSIZE];
			sprintf_s(filePath, BUFSIZE, WWW_ROOT_PATH "%s", req.uri);
			ret = ParseHttpRequestUri(filePath, filePath, NULL, BUFSIZE, 0);
			if (ret < 0) {
				if (ret == ERROR_NOT_IMPLEMENTED) {
					SendHttpHeader(clientSock, HTTP_NOT_IMPLEMENTED, "text/html");
					SendData(clientSock, NOT_IMPLEMENTED_HTML, strlen(NOT_IMPLEMENTED_HTML));
					goto finalize;
				}
			}

			printf("%s %s\n", req.method == REQUEST_GET ? "GET" : "POST", filePath);

			FILE *fp;
			if (fopen_s(&fp, filePath, "r") != 0) {
				SendHttpHeader(clientSock, HTTP_NOT_FOUND, "text/html");
				SendData(clientSock, NOT_FOUND_HTML, strlen(NOT_FOUND_HTML));
				goto finalize;
			}
			fclose(fp);

			SendHttpHeader(clientSock, HTTP_OK, "text/html");
			ret = SendTextFile(clientSock, filePath);
			if (ret < 0) {
				goto finalize;
			}
		}

finalize:
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