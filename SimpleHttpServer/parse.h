#pragma once
#include "http.h"
extern int ParseHttpRequestMessage(const char *message, HttpRequestMessage *structReq);
extern int ParseHttpRequestUri(const char *uri, char *resourcePath, char *args, int pathLen, int argsLen);