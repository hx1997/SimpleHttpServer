#include <stdio.h>
#include "socket.h"
#include "error.h"

#define BUFSIZE 512

int SendHttpHeader(unsigned int clientSock, const char *responseCode, const char *contentType) {
	int datalen;
	char buf[BUFSIZE];

	datalen = sprintf_s(buf, "HTTP/1.0 %s\r\nServer : SimpleHttpServer\r\nConnection : close\r\nContent - Type : %s\r\n\r\n", \
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