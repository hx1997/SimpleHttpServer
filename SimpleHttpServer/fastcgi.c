#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "fastcgi.h"
#include "http.h"
#include "socket.h"
#include "parse.h"
#include "platform.h"

#define BUFSIZE 512u

FCGI_Header GetHeader(int type, int requestId, int contentlen, int paddinglen) {
	FCGI_Header h;
	h.version = FCGI_VERSION_1;
	h.type = (unsigned char)type;
	h.requestIdB0 = (unsigned char)((requestId) & 0xff);
	h.requestIdB1 = (unsigned char)((requestId >> 8) & 0xff);
	h.contentLengthB0 = (unsigned char)((contentlen) & 0xff);
	h.contentLengthB1 = (unsigned char)((contentlen >> 8) & 0xff);
	h.paddingLength = (unsigned char)paddinglen;
	h.reserved = 0;

	return h;
}

FCGI_BeginRequestBody GetBeginRequestBody(int role, int keepConn) {
	FCGI_BeginRequestBody b;
	b.roleB0 = (unsigned char)((role) & 0xff);
	b.roleB1 = (unsigned char)((role >> 8) & 0xff);
	b.flags = (keepConn ? FCGI_KEEP_CONN : 0);
	b.reserved[0] = b.reserved[1] = b.reserved[2] = b.reserved[3] = b.reserved[4] = 0;
	
	return b;
}

int SendBeginRequestRecord(unsigned int clientSock, int requestId) {
	int ret;
	FCGI_BeginRequestRecord beginRecord;

	beginRecord.header = GetHeader(FCGI_BEGIN_REQUEST, requestId, sizeof(beginRecord.body), 0);
	beginRecord.body = GetBeginRequestBody(FCGI_RESPONDER, 0);

	ret = SendData(clientSock, (char *)&beginRecord, sizeof(beginRecord));

	if (ret != sizeof(beginRecord)) {
		return -1;
	}

	return 0;
}

int SendParamRecords(unsigned int clientSock, int requestId, const char *name, int namelen, const char *value, int valuelen) {
	int ret, contentlen, paddinglen;
	contentlen = namelen + valuelen;

	// FastCGI transmits a name-value pair as the length of the name, followed by the length of the value, followed by the name, followed by the value. 
	// Lengths of 127 bytes and less can be encoded in one byte, while longer lengths are always encoded in four bytes
	contentlen += (namelen < 128) ? 1 : 4;
	contentlen += (valuelen < 128) ? 1 : 4;

	// We recommend that records be placed on boundaries that are multiples of eight bytes. The fixed-length portion of a FCGI_Record is eight bytes.
	paddinglen = (contentlen % 8) ? (8 - (contentlen % 8)) : 0;

	char *buf, *bufOrig;
	buf = bufOrig = (char *)malloc(FCGI_HEADER_LEN + contentlen + paddinglen);

	FCGI_Header h = GetHeader(FCGI_PARAMS, requestId, contentlen, paddinglen);
	MEMCPY(buf, FCGI_HEADER_LEN + contentlen + paddinglen, &h, FCGI_HEADER_LEN);
	buf += FCGI_HEADER_LEN;

	if (namelen < 128) {
		*buf++ = (unsigned char)namelen;
	}
	else {
		*buf++ = (unsigned char)((namelen >> 24) | 0x80);
		*buf++ = (unsigned char)(namelen >> 16);
		*buf++ = (unsigned char)(namelen >> 8);
		*buf++ = (unsigned char)namelen;
	}

	if (valuelen < 128) {
		*buf++ = (unsigned char)valuelen;
	}
	else {
		*buf++ = (unsigned char)((valuelen >> 24) | 0x80);
		*buf++ = (unsigned char)(valuelen >> 16);
		*buf++ = (unsigned char)(valuelen >> 8);
		*buf++ = (unsigned char)valuelen;
	}

	MEMCPY(buf, FCGI_HEADER_LEN + contentlen + paddinglen, name, namelen);
	buf += namelen;
	MEMCPY(buf, FCGI_HEADER_LEN + contentlen + paddinglen, value, valuelen);

	ret = SendData(clientSock, bufOrig, FCGI_HEADER_LEN + contentlen + paddinglen);
	free(bufOrig);

	if (ret != FCGI_HEADER_LEN + contentlen + paddinglen) {
		return -1;
	}

	return 0;
}

int SendEmptyRecord(unsigned int clientSock) {
	int ret;
	FCGI_Header h = GetHeader(FCGI_PARAMS, clientSock, 0, 0);

	ret = SendData(clientSock, (char *)&h, FCGI_HEADER_LEN);
	if (ret != FCGI_HEADER_LEN) {
		return -1;
	}

	return 0;
}

int SendStdinRecord(unsigned int clientSock, int requestId, char *data, int datalen) {
	int ret, contentlen = datalen, paddinglen;

	while (datalen > 0) {
		if (datalen > FCGI_MAX_LENGTH) {
			contentlen = FCGI_MAX_LENGTH;
		}

		paddinglen = (datalen % 8) ? (8 - (datalen % 8)) : 0;
		FCGI_Header h = GetHeader(FCGI_STDIN, requestId, contentlen, paddinglen);

		ret = SendData(clientSock, (char *)&h, FCGI_HEADER_LEN);
		if (ret != FCGI_HEADER_LEN) {
			return -1;
		}

		ret = SendData(clientSock, data, contentlen);
		if (ret != contentlen) {
			return -1;
		}

		if (paddinglen > 0) {
			ret = SendData(clientSock, "\0\0\0\0\0\0\0\0", paddinglen);
			if (ret != paddinglen) {
				return -1;
			}
		}

		datalen -= contentlen;
		data += contentlen;
	}

	return 0;
}

int SendEmptyStdinRecord(unsigned int clientSock, int requestId) {
	int ret;

	FCGI_Header h = GetHeader(FCGI_STDIN, requestId, 0, 0);
	ret = SendData(clientSock, (char *)&h, FCGI_HEADER_LEN);
	if (ret != FCGI_HEADER_LEN) {
		return -1;
	}

	return 0;
}

int SendFastCgi(unsigned int clientSock, const char *file, const char *args, char *messageBody, HttpRequestMessage structReq) {
	char *paramNames[] = {
		"SCRIPT_FILENAME",
		//"SCRIPT_NAME",
		"REQUEST_METHOD",
		"REQUEST_URI",
		"QUERY_STRING",
		"CONTENT_TYPE",
		"CONTENT_LENGTH",
	};

	const char *paramValues[] = {
		file,
		GetMethodString(structReq.method),
		structReq.uri,
		args,
		structReq.headers.contenttype,
		structReq.headers.contentlen,
	};

	if (SendBeginRequestRecord(clientSock, clientSock) < 0) {
		return ERROR_FCGI;
	}

	for (int i = 0; i < 6; i++) {
		if (SendParamRecords(clientSock, clientSock, paramNames[i], strlen(paramNames[i]), paramValues[i], strlen(paramValues[i])) < 0) {
			return ERROR_FCGI;
		}
	}

	if (SendEmptyRecord(clientSock) < 0) {
		return ERROR_FCGI;
	}

	int len = atoi(structReq.headers.contentlen);
	if (len > 0) {
		if (SendStdinRecord(clientSock, clientSock, messageBody, len) < 0) {
			return ERROR_FCGI;
		}
	}

	if (SendEmptyStdinRecord(clientSock, clientSock) < 0) {
		return ERROR_FCGI;
	}

	return 0;
}

int SendToCli(unsigned int clientSock, const char *out, int outlen, const char *err, int errlen, FCGI_EndRequestBody *e) {
	int datalen;
	char buf[BUFSIZE];

	datalen = SPRINTF(buf, BUFSIZE, "HTTP/1.0 %s\r\nServer : SimpleHttpServer\r\nConnection : close\r\n", HTTP_OK);
	SendData(clientSock, buf, datalen);

	if (outlen > 0) {
		if (SendData(clientSock, out, outlen) < 0) {
			return -1;
		}
	}

	if (errlen > 0) {
		if (SendData(clientSock, err, errlen) < 0) {
			return -1;
		}
	}

	return 0;
}

int ReceiveRecord(unsigned int httpClientSock, unsigned int cgiClientSock, int requestId) {
	FCGI_Header h;
	FCGI_EndRequestBody e;
	char *outBuf = NULL, *errBuf = NULL;
	int outlen = 0, errlen = 0;
	int ret, contentlen;
	int fcgiRequestId;
	int buf[8];

	while (ReceiveData(cgiClientSock, (char *)&h, FCGI_HEADER_LEN) > 0) {
		fcgiRequestId = (int)(h.requestIdB1 << 8) + (int)(h.requestIdB0);
		if (h.type == FCGI_STDOUT && fcgiRequestId == requestId) {
			contentlen = (int)(h.contentLengthB1 << 8) + (int)(h.contentLengthB0);
			outlen += contentlen;

			if (outBuf != NULL) {
				outBuf = (char *)realloc(outBuf, outlen);
			}
			else {
				outBuf = (char *)malloc(contentlen);
			}

			ret = ReceiveData(cgiClientSock, outBuf, contentlen);
			if (ret != contentlen) {
				free(outBuf);
				return -1;
			}

			if (h.paddingLength > 0) {
				ret = ReceiveData(cgiClientSock, (char *)buf, h.paddingLength);
				if (ret != h.paddingLength) {
					free(outBuf);
					return -1;
				}
			}
		}
		else if (h.type == FCGI_STDERR && fcgiRequestId == requestId) {
			contentlen = (int)(h.contentLengthB1 << 8) + (int)(h.contentLengthB0);
			errlen += contentlen;

			if (errBuf != NULL) {
				errBuf = (char *)realloc(errBuf, errlen);
			}
			else {
				errBuf = (char *)malloc(contentlen);
			}

			ret = ReceiveData(cgiClientSock, errBuf, contentlen);
			if (ret != contentlen) {
				free(errBuf);
				return -1;
			}

			if (h.paddingLength > 0) {
				ret = ReceiveData(cgiClientSock, (char *)buf, h.paddingLength);
				if (ret != h.paddingLength) {
					free(errBuf);
					return -1;
				}
			}
		}
		else if (h.type == FCGI_END_REQUEST && fcgiRequestId == requestId) {
			ret = ReceiveData(cgiClientSock, (char *)&e, sizeof(FCGI_EndRequestBody));
			if (ret != sizeof(FCGI_EndRequestBody)) {
				free(outBuf);
				free(errBuf);
				return -1;
			}

			SendToCli(httpClientSock, outBuf, outlen, errBuf, errlen, &e);
			free(outBuf);
			free(errBuf);
			return 0;
		}
	}

	return 0;
}

int ReceiveFastCgi(unsigned int httpClientSock, unsigned int cgiClientSock) {
	if (ReceiveRecord(httpClientSock, cgiClientSock, cgiClientSock) < 0) {
		return ERROR_FCGI;
	}
	return 0;
}