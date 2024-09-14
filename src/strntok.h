#ifndef __MEMTOK_H
#define __MEMTOK_H

#include <stddef.h>

typedef struct memtok_save_data {
	size_t remaining;
	const char *ptr;
} mt_save_t;

/* TODO: move to a file that makes more sense */
const char *strnstr(const char *haystack, const char *needle, size_t hsl);

char *strntokbuf(const char *in, size_t len, const char *delim, char *out, size_t outlen, mt_save_t *save);

#endif /* __MEMTOK_H */