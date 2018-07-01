#pragma once
typedef struct {
	unsigned short int port;
	char *wwwRootPath;
	char *indexFileName;
} ServerConfig;

// default www root path
#define WWW_ROOT_PATH			"./www"
#define WWW_INDEX_FILE			"index.html"
// default port number
#define PORT_NO			(8080)

extern ServerConfig config;

extern int ParseArguments(int argc, char **argv);