#pragma once
// http response codes
#define HTTP_OK			"200 OK"
#define HTTP_NOT_FOUND			"404 Not Found"
#define HTTP_NOT_IMPLEMENTED			"501 Not Implemented"

// error pages
#define NOT_FOUND_HTML			"<html><head><title>404 Not Found</title></head><body bgcolor = \"white\">\
					<center><h1>404 Not Found</h1></center><hr><center>SimpleHttpServer</center></body></html>"
#define NOT_IMPLEMENTED_HTML			"<html><head><title>501 Not Implemented</title></head><body bgcolor = \"white\">\
					<center><h1>501 Not Implemented</h1></center><hr><center>SimpleHttpServer</center></body></html>"

// only GET request implemented now
enum HttpRequestMethod {
	REQUEST_GET,
	REQUEST_POST,
	REQUEST_HEAD,
	REQUEST_PUT,
	REQUEST_DELETE,
	REQUEST_OPTIONS,
	REQUEST_CONNECT,
	REQUEST_TRACE,
};

typedef struct {
	char *key;
	char *value;
} KeyValuePair;

typedef struct {
	int method;
	char uri[256];
	char version[16];
} HttpRequestMessage;

extern int SendHttpHeader(unsigned int clientSock, const char *responseCode, const char *contentType);
extern int ServeStatic(unsigned int clientSock, const char *file, size_t size);