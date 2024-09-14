#ifndef __SERVER_H
#define __SERVER_H

struct server_info {
	char *host;
	char *root;
	char *user;
	unsigned short port;
};

#endif /* __SERVER_H */