#pragma once
// http response codes
#define HTTP_OK			"200 OK"
#define HTTP_NOT_FOUND			"404 Not Found"
#define HTTP_INTERNAL_SERVER_ERROR			"500 Internal Server Error"
#define HTTP_NOT_IMPLEMENTED			"501 Not Implemented"

// error pages
#define NOT_FOUND_HTML			"<html><head><title>404 Not Found</title></head><body bgcolor = \"white\">\
					<center><h1>404 Not Found</h1></center><hr><center>SimpleHttpServer</center></body></html>"
#define INTERNAL_SERVER_ERROR_HTML			"<html><head><title>500 Internal Server Error</title></head><body bgcolor = \"white\">\
					<center><h1>500 Internal Server Error</h1></center><hr><center>SimpleHttpServer</center></body></html>"
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
	struct {
		char contenttype[256];
		char contentlen[16];
	} headers;
} HttpRequestMessage;

extern int SendHttpHeader(unsigned int clientSock, const char *responseCode, const char *contentType);
extern void SendError(unsigned int clientSock, const char *responseCode, const char *errorPageHtml);
extern int ServeStatic(unsigned int clientSock, const char *file, size_t size);
extern int ServeDynamic(unsigned int clientSock, const char *file, const char *args, char *messageBody, HttpRequestMessage structReq);