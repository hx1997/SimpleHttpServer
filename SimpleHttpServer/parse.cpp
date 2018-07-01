#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "http.h"

int ParseHttpRequestMethod(const char *method) {
	int len = strlen(method);

	switch (len)
	{
	case 3:
		if (strncmp(method, "GET", 3) == 0) {
			return REQUEST_GET;
		}
		break;
	case 4:
		if (strncmp(method, "POST", 4) == 0) {
			return REQUEST_POST;
		}
		break;
	default:
		return ERROR_NOT_IMPLEMENTED;
	}

	return ERROR_NOT_IMPLEMENTED;
}

int ParseHttpRequestLine(const char *requestLine, HttpRequestMessage *structReq) {
	int ret;
	char method[16];

	sscanf_s(requestLine, "%s %s %s\r\n", method, 16, structReq->uri, 256, structReq->version, 16);
	if ((ret = ParseHttpRequestMethod(method)) < 0) {
		return ret;
	}

	structReq->method = ret;
	return 0;
}

// @return
// 0 - static
// 1 - dynamic
int ParseHttpRequestUri(const char *uri, char *resourcePath, char *args, int pathLen, int argsLen) {
	char uriCopy[256];
	strncpy_s(uriCopy, uri, 256);
	int ret = 0;

	_strlwr_s(uriCopy + strlen(uriCopy) - 4, 5);
	if (strncmp(uriCopy + strlen(uriCopy) - 4, ".php", 4) == 0) {
		ret = 1;
		return ERROR_NOT_IMPLEMENTED;
	}

	const char *ch = uriCopy;

	// assume index page if uri ends in a slash
	if (*(ch + strlen(ch) - 1) == '/') {
		strncat_s(uriCopy, 256, "index.html", 10);
	}

	// parse arguments
	while (1) {
		if (*ch == '?') {
			strncpy_s(resourcePath, pathLen, uriCopy, ch - uriCopy);
			if (args) strncpy_s(args, argsLen, ++ch, uriCopy + strlen(uriCopy) - 1 - ch);
			return ret;
		}
		else if (*ch == '\0') {
			break;
		}
		++ch;
	}

	strncpy_s(resourcePath, pathLen, uriCopy, 256);
	return ret;
}

// TODO: check if message is of a valid HTTP request,
// current implementation works incorrectly if it is not
int ParseHttpRequestMessage(const char *message, HttpRequestMessage *structReq) {
	int ret;

	ret = ParseHttpRequestLine(message, structReq);
	if (ret < 0) {
		return ret;
	}

	return 0;
}