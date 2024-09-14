#ifndef __KEYVAL_H
#define __KEYVAL_H

#include <stddef.h>

struct keyval {
	char *key;
	char *value;
};

#define keyval_iter(__pair, __pairs) \
	for (struct keyval *__pair = __pairs; __pair->key && __pair->value; __pair++)

struct keyval *keyval_alloc();

void cleanup_keyvals(struct keyval *pairs);

void free_keyvals(struct keyval *pairs);

struct keyval *kv_get_mut_pair(struct keyval *pairs, const char *key);

const struct keyval *kv_get_pair(struct keyval *pairs, const char *key);

char *kv_get_mut_value(struct keyval *pairs, const char *key);

const char *kv_get_value(struct keyval *pairs, const char *key);

struct keyval *kv_set_value(struct keyval *pairs, const char *key, const char *value);

struct keyval *kv_clear_key(struct keyval *pairs, const char *key);

#endif /* __KEYVAL_H */
