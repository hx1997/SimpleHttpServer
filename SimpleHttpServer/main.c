#include <stdio.h>
#include <string.h>
#include "config.h"
#include "socket.h"
#include "parse.h"
#include "error.h"
#include "platform.h"

#define BUFSIZE			8192u

static void usage(const char *executable) {
	printf("Usage: %s [-p port] [-r www_root] [-i index_filename]\n", executable);
}

int main(int argc, char **argv) {
	int ret;

	if (argc >= 2) {
		if (ParseArguments(argc, argv) < 0) {
			usage(argv[0]);
			return ERROR_INVALID_ARGUMENTS;
		}
	}

	// init winsock
#ifdef _WIN32
	ret = InitWinSock();
	if (ret < 0) {
		return ret;
	}
#endif

	printf("SimpleHttpServer started.\n");
	
	unsigned int sock = ListenForHttpConnection(config.port);
	if ((signed int)sock == -1) {
		return (signed int)sock;
	}
	printf("Listening on port %d for incoming HTTP connections.\n", config.port);
	printf("WWW Root: %s\n", config.wwwRootPath);
	printf("Index file: %s\n", config.indexFileName);

	while (1) {
		struct sockaddr_in clientSockaddrIn = { 0 };
		// wait for connection
		unsigned int clientSock = AcceptConnection(sock, &clientSockaddrIn, sizeof(clientSockaddrIn));
		if ((signed int)clientSock == -1) {
			CloseSocket(sock);
			return (signed int)clientSock;
		}

		printf("\nIncoming connection\n");
		printf("Client socket = %s:%d\n", inet_ntoa(clientSockaddrIn.sin_addr), clientSockaddrIn.sin_port);
		
		char recvBuf[BUFSIZE] = { 0 };

		ret = ReceiveData(clientSock, recvBuf, BUFSIZE);
		if (ret > 0) {
			// parse request message; return error if request method is not yet implemented
			HttpRequestMessage req = { 0 };
			char *messageBody;
			ret = ParseHttpRequestMessage(recvBuf, &req, &messageBody);
			if (ret < 0) {
				if (ret == ERROR_NOT_IMPLEMENTED) {
					SendError(clientSock, HTTP_NOT_IMPLEMENTED, NOT_IMPLEMENTED_HTML);
					goto finalize;
				}
			}

			// parse request uri; return error if php file is being requested (not supported yet)
			char fullUri[BUFSIZE], filePath[BUFSIZE], params[BUFSIZE];
			SPRINTF(fullUri, BUFSIZE, "%s", config.wwwRootPath);
			SPRINTF(fullUri, BUFSIZE, "%s%s", fullUri, req.uri);
			ret = ParseHttpRequestUri(fullUri, filePath, params, BUFSIZE, BUFSIZE);
			if (ret < 0) {
				if (ret == ERROR_NOT_IMPLEMENTED) {
					SendError(clientSock, HTTP_NOT_IMPLEMENTED, NOT_IMPLEMENTED_HTML);
					goto finalize;
				}
			}

			printf("%s %s\n", req.method == REQUEST_GET ? "GET" : "POST", filePath);

			// requested file exists?
			FILE *fp;
			if (fopenPortable(&fp, filePath, "r") != 0) {
				SendError(clientSock, HTTP_NOT_FOUND, NOT_FOUND_HTML);
				goto finalize;
			}
			fclose(fp);

			if (ret == 0) {
				// serve static content
				if (ServeStatic(clientSock, filePath, BUFSIZE) < 0) {
					goto finalize;
				}
			}
			else {
				// serve dynamic content
				if (ServeDynamic(clientSock, filePath, params, messageBody, req) < 0) {
					goto finalize;
				}
			}
		}

finalize:
		ShutdownSocket(clientSock);
		CloseSocket(clientSock);
	}

	CloseSocket(sock);

	// cleanup winsock
#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}