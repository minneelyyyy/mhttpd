#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "server.h"

/* info must be properly filled out and sock must be a correctly initialized listen socket */
extern int httpd(struct server_info *info, int sock);

static int daemonize()
{
	pid_t pid;

	pid = fork();

	if (pid < 0) {
		fprintf(stderr, "error: failed to spawn daemon (exorcism success?): %s\n", strerror(errno));
		exit(0);
	}

	return pid == 0;
}

int main(int argc, char **argv)
{
	struct server_info info = { NULL, NULL, NULL, 8080 };
	int c, sock, uid, one = 1;
	struct sockaddr_in addr;

	while ((c = getopt(argc, argv, "p:h:r:u:d")) != EOF) {
		switch (c) {
		case 'h':
			info.host = strdup(optarg);
			break;
		case 'r':
			info.root = strdup(optarg);
			break;
		case 'p':
			info.port = atoi(optarg);
			break;
		case 'u':
			info.user = strdup(optarg);
			break;
		case 'd':
		        if (daemonize()) return 0;
		}
	}

	if (info.user) {
		char *buf;
		size_t bufsz;
		struct passwd pwd, *result;
		int s;
		
		bufsz = sysconf(_SC_GETPW_R_SIZE_MAX);
		bufsz = bufsz > -1 ? bufsz : 16384;

		buf = malloc(bufsz);

		s = getpwnam_r(info.user, &pwd, buf, bufsz, &result);

		if (result == NULL) {
			if (s) {
				fprintf(stderr, "error: failed to get user: %s\n", strerror(s));
				return 1;
			} else {
				fprintf(stderr, "error: user appears to not exist\n");
				return 1;
			}
		}

		uid = pwd.pw_uid;

		free(buf);
	} else {
		fprintf(stderr, "warning: specifying an unpriviledged user to run the server as is highly recommended.\n");
	}

	if (!info.root) {
		long size;

		fprintf(stderr, "warning: no root directory was specified, using the current working directory\n");

		size = pathconf(".", _PC_PATH_MAX);
		size = size > 0 ? size : 16384;
		info.root = malloc(size);

		if (!getcwd(info.root, size)) {
			fprintf(stderr, "error: failed to get CWD: %s\n", strerror(errno));
			return 1;
		}
	}

	if (chroot(info.root) != 0) {
		fprintf(stderr, "error: failed to chroot into %s: %s\n", info.root, strerror(errno));
		return 1;
	}

	if (info.user) {
		if (setuid(uid) != 0) {
			fprintf(stderr, "error: failed to set user: %s\n", strerror(errno));
			return 1;
		}
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(info.port);
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) != 0) {
		fprintf(stderr, "error: could not set REUSEADDR for socket: %s\n", strerror(errno));
		return 1;
	}

	if ((bind(sock, (struct sockaddr*) &addr, sizeof(addr))) != 0) {
		fprintf(stderr, "error: failed to bind socket: %s\n", strerror(errno));
		return 1;
	}

	if ((listen(sock, 16)) != 0) {
		fprintf(stderr, "error: failed to listen on socket: %s\n", strerror(errno));
		return 1;
	}

	return httpd(&info, sock);
}
