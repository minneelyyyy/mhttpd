#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#include "http.h"
#include "strntok.h"

static ssize_t parse_request_line(struct http_request *request,
		char *buffer, size_t buffersz, char *errbuf, size_t errbufsz) {
	char method[256];
	char version[256];
	mt_save_t save;

	if (!strnstr(buffer, "\r\n", buffersz)) {
		snprintf(errbuf, errbufsz, "no newline found in buffer, cannot parse a request line");
		return -1;
	}

	strntokbuf(buffer, buffersz, " ", method, sizeof(method), &save);
	strntokbuf(NULL, 0, " ", request->path, sizeof(request->path), &save);
	strntokbuf(NULL, 0, "\r\n", version, sizeof(version), &save);

	request->method = http_method_from_str(method);
	request->version = http_version_from_str(version);

	return strlen(method) + strlen(request->path) + strlen(version) + 4;
}

static const char *trim_start(const char *s) {
	const char *c;

	if (!s)
		return NULL;

	for (c = s; *c; c++)
		if (*c == ' ' || *c == '\t' || *c == '\r' || *c == '\n')
			continue;
		else
			return c;

	return c;
}

/** re-entrant header parser
 * Takes a buffer of headers and adds them into a keyval array.
 * If it does not encounter an end of headers line ("\r\n"), then it returns the
 * number of used bytes and sets state to be PARSE_STATE_INCOMPLETE.
 * When it reads all possible headers, it returns 0 and state is set to PARSE_STATE_COMPLETE.
 * If an error occurs, -1 is returned and state is PARSE_STATE_ERROR.
 */
static ssize_t parse_headers(struct keyval *headers, int *state, char *buffer, size_t buffersz,
		char *errbuf, size_t errbufsz) {
	char *s;
	char token[4096];
	ssize_t used = 0;
	mt_save_t save;

	if (!strnstr(buffer, "\r\n", buffersz)) {
		snprintf(errbuf, errbufsz, "no end of line found in buffer, a header may be too long");
		*state = PARSE_STATE_ERROR;
		return -1;
	}

	strntokbuf(buffer, buffersz, "\r\n", token, sizeof(token), &save);

	do {
		char *key, *value;
		char *s1;

		if (!strcmp(token, "\r\n")) {
			*state = PARSE_STATE_FINISHED;
			return 0;
		}

		key = strtok_r(token, ":", &s1);
		value = strtok_r(NULL, "", &s1);

		if (!key) {
			snprintf(errbuf, errbufsz, "header is missing a colon deliminater");
			*state = PARSE_STATE_ERROR;
			return -1;
		}

		if (!(value = (char*) trim_start(value))) {
			snprintf(errbuf, errbufsz, "header \"%s\" is missing a value", key);
			*state = PARSE_STATE_ERROR;
			return -1;
		}

		kv_set_value(headers, key, value);

		used += strlen(token) + 2;
	} while (strntokbuf(NULL, 0, "\r\n", token, sizeof(token), &save));

	*state = PARSE_STATE_INCOMPLETE;
	return used;
}

ssize_t parse_http_request(struct http_request *request, int *state, char *buffer, size_t buffersz,
		char *errbuf, size_t errbufsz) {
	ssize_t used = 0;

	switch (*state) {
	case PARSE_STATE_NO_PROGRESS:
		if ((used = parse_request_line(request, buffer, buffersz, errbuf, errbufsz)) < 0)
			goto err;
	case PARSE_STATE_INCOMPLETE: {
		ssize_t r = parse_headers(request->headers, state, buffer + used, buffersz - used, errbuf, errbufsz);

		if (r < 0) {
			used = -1;
			goto err;
		}

		used += r;

		if (*state & (PARSE_STATE_INCOMPLETE | PARSE_STATE_FINISHED))
			memmove(buffer, buffer + used, buffersz - used);
	}
	break;
	default:
		snprintf(errbuf, errbufsz, "invalid state");
		used = -1;
	case PARSE_STATE_FINISHED:
		goto err;
	}

err:
	return used;
}

int serialize_http_response(const struct http_response *response, int stream) {
	char buffer[8192];
	int bytes;

	bytes = snprintf(buffer, sizeof(buffer), "%s %.256s %d\r\n",
		http_version_get_str(response->version), response->message, response->code);
	write(stream, buffer, bytes);

	keyval_iter(pair, response->headers) {
		bytes = snprintf(buffer, sizeof(buffer), "%s: %s\r\n", pair->key, pair->value);
		write(stream, buffer, bytes);
	}

	write(stream, "\r\n", 2);

	return 0;
}
