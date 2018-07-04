#include <stdio.h>
#include <string.h>
#include "http.h"
#include "fastcgi.h"
#include "socket.h"
#include "error.h"
#include "parse.h"
#include "platform.h"

#define BUFSIZE 512u

int SendHttpHeader(unsigned int clientSock, const char *responseCode, const char *contentType) {
	int datalen;
	char buf[BUFSIZE];

	datalen = SPRINTF(buf, BUFSIZE, "HTTP/1.0 %s\r\nServer : SimpleHttpServer\r\nConnection : close\r\nContent-Type : %s\r\n\r\n", \
		responseCode, contentType);

	return SendData(clientSock, buf, datalen);
}

int SendTextFile(unsigned int clientSock, const char *file) {
	int ret;
	FILE *fp;
	char buf[BUFSIZE];

	if (fopenPortable(&fp, file, "r") != 0) {
		return ERROR_FOPEN;
	}

	while (!feof(fp)) {
		int datalen = FREAD(buf, BUFSIZE, 1, BUFSIZE, fp);
		ret = SendData(clientSock, buf, datalen);
		if (ret < 0) {
			fclose(fp);
			return ret;
		}
	}

	fclose(fp);
	return 0;
}

void SendError(unsigned int clientSock, const char *responseCode, const char *errorPageHtml) {
	SendHttpHeader(clientSock, responseCode, "text/html");
	SendData(clientSock, errorPageHtml, strlen(errorPageHtml));
}

int ServeStatic(unsigned int clientSock, const char *file, size_t bufsize) {
	int ret;

	// determine content type and send it in header
	ret = GetMimeType(file, bufsize);
	if (ret >= 0) {
		SendHttpHeader(clientSock, HTTP_OK, mimeTypeMap[ret][1]);
	}
	else {
		SendHttpHeader(clientSock, HTTP_OK, "text/html");
	}

	// send http message body
	ret = SendTextFile(clientSock, file);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int ServeDynamic(unsigned int clientSock, const char *file, const char *args, char *messageBody, HttpRequestMessage structReq) {
	int ret;
	unsigned int cgiClientSock = OpenCGIClientSock();

	if ((signed int)cgiClientSock == -1) {
		SendError(clientSock, HTTP_INTERNAL_SERVER_ERROR, INTERNAL_SERVER_ERROR_HTML);
		return ERROR_SOCKET;
	}

	ret = SendFastCgi(cgiClientSock, file, args, messageBody, structReq);
	if (ret < 0) {
		SendError(clientSock, HTTP_INTERNAL_SERVER_ERROR, INTERNAL_SERVER_ERROR_HTML);
		return ret;
	}

	ret = ReceiveFastCgi(clientSock, cgiClientSock);
	if (ret < 0) {
		SendError(clientSock, HTTP_INTERNAL_SERVER_ERROR, INTERNAL_SERVER_ERROR_HTML);
		return ret;
	}

	CLOSESOCKET(cgiClientSock);

	return 0;
}