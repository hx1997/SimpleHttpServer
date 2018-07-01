#include <stdio.h>
#include "http.h"
#include "socket.h"
#include "error.h"
#include "parse.h"
#include "platform.h"

#define BUFSIZE 512

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