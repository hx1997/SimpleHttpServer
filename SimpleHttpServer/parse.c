#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "error.h"
#include "http.h"
#include "parse.h"
#include "platform.h"

#define minimum(a, b)			((a) < (b) ? (a) : (b))
#define BUFSIZE			8192u

const char *mimeTypeMap[][2] = {
	{ "html", "text/html" },
	{ "js", "application/javascript" },
	{ "css", "text/css" },
	{ "txt", "text/plain" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ NULL, NULL },
};

char *strlwr_n(char *str)
{
	unsigned char *p = (unsigned char *)str;

	while (*p) {
		*p = (unsigned char)tolower(*p);
		p++;
	}

	return str;
}

int GetMimeType(const char *file, size_t size) {
	size_t len = minimum(strlen(file), size);
	char *buf = (char *)malloc(len + 1);

	//if (*(buf + len - 4) != '.' && *(buf + len - 5) != '.') {
	//	return -1;
	//}

	STRNCPY(buf, len + 1, file, len);
	strlwr_n(buf + len - 4);
	for (int i = 0; mimeTypeMap[i][0]; i++) {
		size_t extension_len = strlen(mimeTypeMap[i][0]);
		if (strncmp(buf + len - extension_len, mimeTypeMap[i][0], extension_len) == 0) {
			free(buf);
			return i;
		}
	}

	free(buf);
	return -1;
}

char *GetMethodString(int method) {
	switch (method)
	{
	case 0:
		return "GET";
	case 1:
		return "POST";
	}

	return NULL;
}

int ParseHttpRequestMethod(const char *method, size_t size) {
	size_t len = minimum(strlen(method), size);

	switch (len)
	{
	case 3:
		if (strncmp(method, "GET", 3u) == 0) {
			return REQUEST_GET;
		}
		break;
	case 4:
		if (strncmp(method, "POST", 4u) == 0) {
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

	// TODO: More portable fix
#ifdef _WIN32
	sscanf_s(requestLine, "%s %s %s\r\n", method, 16, structReq->uri, 256, structReq->version, 16);
#else
	sscanf(requestLine, "%s %s %s\r\n", method, structReq->uri, structReq->version);
#endif // WIN32

	if ((ret = ParseHttpRequestMethod(method, 16u)) < 0) {
		return ret;
	}

	structReq->method = ret;
	return 0;
}

char *ResolveRelativePath(const char *relativePath, char *absolutePath, size_t size) {
#ifdef WIN32
	return _fullpath(absolutePath, relativePath, size);
#else
	return realpath(relativePath, absolutePath);
#endif // WIN32
}

// @return
// 0 - static
// 1 - dynamic
int ParseHttpRequestUri(char *uri, char *resourcePath, char *params, size_t pathLen, size_t argsLen) {
	char uriCopy[BUFSIZE];
	STRNCPY(resourcePath, BUFSIZE, uri, strlen(uri));
	int ret = 0;

	char *ch = uri;
	// parse http parameters in uri
	while (1) {
		if (*ch == '?') {
			resourcePath[ch - uri] = '\0';

			if (params) {
				++ch;
				STRNCPY(params, argsLen, ch, uri + strlen(uri) - ch);
			}

			break;
		}
		else if (*ch == '\0') {
			break;
		}
		++ch;
	}

	ch = resourcePath;
	
	// assume index page if uri ends in a slash
	if (*(ch + strlen(ch) - 1) == '/') {
		STRNCAT(ch, BUFSIZE, config.indexFileName, strlen(config.indexFileName));
	}

	strlwr_n(ch + strlen(ch) - 4);

	// return 1 if php file is being requested
	if (strncmp(ch + strlen(ch) - 4, ".php", 4u) == 0) {
		ret = 1;
	}

	// FastCGI doesn't take relative path so we need to resolve it
	if (ret) {
		ResolveRelativePath(resourcePath, uriCopy, pathLen);
		STRNCPY(resourcePath, BUFSIZE, uriCopy, strlen(uriCopy));
	}

	return ret;
}

void ExtractContentLength(const char *str, HttpRequestMessage *structReq) {
	const char *ch = strstr(str, ":") + 1;
	while (*ch == ' ' && *ch != '\0') ch++;

	const char *end = ch;
	while (*end != '\r' && *end != '\0') end++;

	if (*(end + 1) != '\n') {
		STRNCPY(structReq->headers.contentlen, 16u, "", 0);
	}

	STRNCPY(structReq->headers.contentlen, 16u, ch, end - ch);
	structReq->headers.contentlen[end - ch] = '\0';
}

void ExtractContentType(const char *str, HttpRequestMessage *structReq) {
	const char *ch = strstr(str, ":") + 1;
	while (*ch == ' ' && *ch != '\0') ch++;

	const char *end = ch;
	while (*end != '\r' && *end != '\0') end++;

	if (*(end + 1) != '\n') {
		STRNCPY(structReq->headers.contenttype, 256u, "", 0);
	}

	STRNCPY(structReq->headers.contenttype, 256u, ch, end - ch);
	structReq->headers.contenttype[end - ch] = '\0';
}

int ParseHttpRequestHeaders(const char *headerLines, HttpRequestMessage *structReq) {
	char lineCopy[BUFSIZE];
	const char *ch = lineCopy;
	STRNCPY(lineCopy, BUFSIZE, headerLines, strlen(headerLines));
	strlwr_n(lineCopy);

	char *messageBodyStart = strstr(lineCopy, "\r\n\r\n");

	for ( ; ch < messageBodyStart; ) {
		if (strncmp(ch, "content-", 8) == 0) {
			ch += 8;
			if (strncmp(ch, "type", 4) == 0) {
				ch += 4;
				ExtractContentType(ch, structReq);
			}
			else if (strncmp(ch, "length", 6) == 0) {
				ch += 6;
				ExtractContentLength(ch, structReq);
			}
		}
		// skip to next line
		while (*ch != '\r' && *ch != '\0') ch++;
		ch += 2;			// skip '\r\n'
	}

	return 0;
}

// TODO: check if message is of a valid HTTP request,
// current implementation works incorrectly (and unsafely) if it is not
int ParseHttpRequestMessage(char *message, HttpRequestMessage *structReq, char **messageBodyStart) {
	int ret;

	ret = ParseHttpRequestLine(message, structReq);
	if (ret < 0) {
		return ret;
	}

	// skip to header lines
	while (*message != '\r' && *message != '\0') message++;
	// TODO: Fix potential buffer overflow vulnerability
	message += 2;		// skip '\r\n'

	ret = ParseHttpRequestHeaders(message, structReq);
	if (ret < 0) {
		return ret;
	}

	*messageBodyStart = strstr(message, "\r\n\r\n") + 4;

	return 0;
}