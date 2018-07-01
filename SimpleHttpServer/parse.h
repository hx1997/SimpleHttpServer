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
extern int ParseHttpRequestMessage(const char *message, HttpRequestMessage *structReq);
extern int ParseHttpRequestUri(char *uri, char *resourcePath, char *args, int pathLen, int argsLen);