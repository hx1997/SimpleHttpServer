#pragma once
typedef struct {
	unsigned short int port;
	char *wwwRootPath;
	char *indexFileName;
	unsigned short int fcgiPort;
	char *fcgiHost;
	char enableMultiprocessing;
} ServerConfig;

// default www root path
#define WWW_ROOT_PATH			"./www"
#define WWW_INDEX_FILE			"index.html"
// default port number
#define PORT_NO			(8080)
#define FCGI_PORT			(9000)
// default CGI Host
#define FCGI_HOST			"127.0.0.1"

extern ServerConfig config;

extern int ParseArguments(int argc, char **argv);