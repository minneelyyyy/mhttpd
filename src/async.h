#ifndef __ASYNC_H
#define __ASYNC_H

#include <server.h>

int mhttpd_async_loop(const struct server_info *info, int sock);

#endif /* __ASYNC_H */