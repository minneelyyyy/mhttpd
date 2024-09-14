#ifndef __HTTP_H
#define __HTTP_H

#include "keyval.h"

enum http_version {
	HTTP11,
	HTTP2,
	HTTP3,
	VERSION_NONE,
};

static inline const char *http_version_get_str(enum http_version version) {

	switch (version) {
		case HTTP11: return "HTTP/1.1";
		case HTTP2:  return "HTTP/2";
		case HTTP3:  return "HTTP/3";
		default:     return "(unknown)";
	}
}

static inline enum http_version http_version_from_str(const char *str) {
	if (!strcmp(str, "HTTP/1.1"))
		return HTTP11;
	if (!strcmp(str, "HTTP/2"))
		return HTTP2;
	if (!strcmp(str, "HTTP/3"))
		return HTTP3;

	return VERSION_NONE;
}

enum http_method {
	METHOD_NONE,
	GET,
	POST,
};

static inline const char *http_method_get_str(enum http_method method) {
	switch (method) {
		case GET:  return "GET";
		case POST: return "POST";
		default:   return "(unknown)";
	}
}

static inline enum http_method http_method_from_str(const char *str) {
	if (!strcmp(str, "GET"))
		return GET;
	if (!strcmp(str, "POST"))
		return POST;
	
	return METHOD_NONE;
}

struct http_request {
	enum http_method method;
	char path[1024];
	enum http_version version;
	struct keyval *headers;
};

struct http_response {
	enum http_version version;
	char message[256];
	int code;
	struct keyval *headers;
};

#define PARSE_STATE_NO_PROGRESS 0x01
#define PARSE_STATE_INCOMPLETE 0x02
#define PARSE_STATE_FINISHED 0x04
#define PARSE_STATE_ERROR 0x08

/** re-entrant http header parser
 * limitations: the request-line and all headers must fit into a single buffer
 */
ssize_t parse_http_request(struct http_request *request, int *state, char *buffer, size_t buffersz,
		char *errbuf, size_t errbufsz);

void cleanup_http_request(struct http_request *request);

int serialize_http_response(const struct http_response *response, int stream);

#endif /* __HTTP_H */