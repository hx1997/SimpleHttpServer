#pragma once
#include "http.h"

typedef enum {
	MIME_HTML,
	MIME_JAVASCRIPT,
	MIME_CSS,
	MIME_TEXT,
	MIME_GIF,
	MIME_JPEG,
	MIME_PNG,
} MimeType;

extern const char *mimeTypeMap[][2];

extern int GetMimeType(const char *file, size_t size);
extern char *GetMethodString(int method);
extern int ParseHttpRequestMessage(char *message, HttpRequestMessage *structReq, char **messageBodyStart);
extern int ParseHttpRequestUri(char *uri, char *resourcePath, char *args, size_t pathLen, size_t argsLen);