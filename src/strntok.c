#include <stdio.h>
#include <string.h>

#include "strntok.h"

const char *strnstr(const char *haystack, const char *needle, size_t hsl) {
	size_t i;
	size_t ndl = strlen(needle);

	hsl = strnlen(haystack, hsl);

	for (i = 0; i < hsl - ndl; i++) {
		if (!memcmp(haystack + i, needle, ndl))
			return haystack + i;
	}

	return NULL;
}

char *strntokbuf(const char *in, size_t len, const char *delim, char *out, size_t outlen, mt_save_t *save) {
	const char *found;

	if (in) {
		save->ptr = in;
		save->remaining = len;
	}

	if (save->remaining == 0)
		return NULL;

	found = strnstr(save->ptr, delim, save->remaining);

	if (!found) {
		snprintf(out, outlen, "%.*s", (int) save->remaining, save->ptr);
		save->remaining = 0;
		return out;
	}

	snprintf(out, outlen, "%.*s", (int)(found - save->ptr), save->ptr);

	save->remaining -= (found - save->ptr) + strlen(delim);
	save->ptr = found + strlen(delim);

	return out;
}

#ifdef TEST
#include <assert.h>
#include <string.h>

int main(void) {
	char buf[] = "ab:cd:ef:gh";
	char data[512];
	mt_save_t save;

	assert(strnstr("abcdefghijklmnopqrstuvwxyz", "g", 6) == NULL);
	assert(strnstr("abcdefghijklmnopqrstuvwxyz", "b", 6));

	assert(!strcmp(strntokbuf(buf, sizeof(buf), ":", data, sizeof(data), &save), "ab"));
	assert(!strcmp(strntokbuf(NULL, 0, ":", data, sizeof(data), &save), "cd"));
	assert(!strcmp(strntokbuf(NULL, 0, ":", data, sizeof(data), &save), "ef"));
	assert(!strcmp(strntokbuf(NULL, 0, ":", data, sizeof(data), &save), "gh"));
	assert(strntokbuf(NULL, 0, ":", data, sizeof(data), &save) == NULL);

	return 0;
}
#endif