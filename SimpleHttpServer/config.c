#include <stdlib.h>
#include "error.h"
#include "config.h"

ServerConfig config = {
	PORT_NO,			// port
	WWW_ROOT_PATH,			// www root directory
	WWW_INDEX_FILE,			// www index file name
	FCGI_PORT,			// Fast-CGI port
	FCGI_HOST,			// Fast-CGI host
};

int ParseArguments(int argc, char **argv) {
	if (argc < 2) {
		return ERROR_TOO_FEW_ARGUMENTS;
	}

	for (int i = 1; i < argc; i++) {
		char *ch = argv[i];

		if (*ch != '-') {
			return ERROR_INVALID_ARGUMENTS;
		}
		else {
			ch++;
		}

		switch (*ch)
		{
		case 'p':
			if (argv[++i]) {
				config.port = (unsigned short int)atoi(argv[i]);
			}
			else {
				return ERROR_INVALID_ARGUMENTS;
			}
			break;
		case 'r':
			if (argv[++i]) {
				config.wwwRootPath = argv[i];
			}
			else {
				return ERROR_INVALID_ARGUMENTS;
			}
			break;
		case 'i':
			if (argv[++i]) {
				config.indexFileName = argv[i];
			}
			else {
				return ERROR_INVALID_ARGUMENTS;
			}
			break;
		case 'f':
			if (argv[++i]) {
				config.fcgiPort = (unsigned short int)atoi(argv[i]);
			}
			else {
				return ERROR_INVALID_ARGUMENTS;
			}
			break;
		case 'h':
			if (argv[++i]) {
				config.fcgiHost = argv[i];
			}
			else {
				return ERROR_INVALID_ARGUMENTS;
			}
			break;
		default:
			return ERROR_INVALID_ARGUMENTS;
			break;
		}
	}

	return 0;
}