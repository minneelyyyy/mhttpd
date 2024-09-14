#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/sendfile.h>

#include "server.h"
#include "http.h"

int httpd(struct server_info *info, int sock) {
	char errbuf[4096];
	int conn;

	while ((conn = accept(sock, NULL, NULL)) > 0) {
		struct http_request req;
		int state = PARSE_STATE_NO_PROGRESS;
		ssize_t used = 0, r = 0;

		req.headers = keyval_alloc();

		while (!(state & (PARSE_STATE_FINISHED | PARSE_STATE_ERROR))) {
			char buffer[8192];

			if ((r = read(conn, buffer + (r - used), sizeof(buffer) - (r - used))) < 0) {
				fprintf(stderr, "error: failed to read: %s\n", strerror(errno));
				goto looperr;
			}

			used = parse_http_request(&req, &state, buffer, r + used, errbuf, sizeof(errbuf));
		}

		if (state & PARSE_STATE_ERROR) {
			fprintf(stderr, "parsing failed: %s\n", errbuf);
			goto looperr;
		}

		printf(	"Method: %s\n"
				"File: %.1024s\n"
				"Version: %s\n",
			http_method_get_str(req.method),
			req.path,
			http_version_get_str(req.version)
		);

		keyval_iter(header, req.headers) {
			printf("%s: %s\n", header->key, header->value);
		}

looperr:
		free_keyvals(req.headers);
		close(conn);
	}

	return 0;
}
