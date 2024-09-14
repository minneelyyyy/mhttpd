#pragma testing

#include <stdlib.h>
#include <string.h>

#include "keyval.h"

struct keyval_pair_header {
	size_t capacity;
};

#define KV_GET_HEADER(__kv) (((struct keyval_pair_header*) (__kv)) - 1)
#define KV_GET_PAIRS(__hdr) ((struct keyval*)((__hdr) + 1))

struct keyval *keyval_alloc() {
	struct keyval_pair_header *hdr = malloc(sizeof(struct keyval_pair_header) + sizeof(struct keyval));
	struct keyval *pairs;

	hdr->capacity = 0;

	pairs = KV_GET_PAIRS(hdr);

	pairs[0].key = NULL;
	pairs[0].value = NULL;

	return pairs;
}

static struct keyval *resize(struct keyval *pairs, size_t new_capacity) {
	struct keyval_pair_header *hdr = KV_GET_HEADER(pairs);
	void *new_block = realloc(hdr, sizeof(struct keyval_pair_header) + sizeof(struct keyval) * (new_capacity + 1));
	size_t i;

	if (!new_block)
		return NULL;

	hdr = (struct keyval_pair_header*) new_block;
	pairs = KV_GET_PAIRS(hdr);

	for (i = hdr->capacity; i <= new_capacity; i++) {
		pairs[i].key = NULL;
		pairs[i].value = NULL;
	}

	hdr->capacity = new_capacity;

	return pairs;
}

void cleanup_keyvals(struct keyval *pairs) {
	struct keyval *pair;

	if (!pairs)
		return;

	for (pair = pairs; pair->key; pair++) {
		free(pair->key);
		free(pair->value);
	}
}

void free_keyvals(struct keyval *pairs) {
	struct keyval_pair_header *hdr;

	if (!pairs)
		return;

	hdr = KV_GET_HEADER(pairs);

	cleanup_keyvals(pairs);

	free(hdr);
}

struct keyval *kv_get_mut_pair(struct keyval *pairs, const char *key) {
	struct keyval *pair;

	for (pair = pairs; pair->key; pair++)
		if (!strcmp(pair->key, key))
			return pair;

	return NULL;
}

char *kv_get_mut_value(struct keyval *pairs, const char *key) {
	struct keyval *pair = kv_get_mut_pair(pairs, key);
	return pair ? pair->value : NULL;
}

const struct keyval *kv_get_pair(struct keyval *pairs, const char *key) {
	return kv_get_mut_pair(pairs, key);
}

const char *kv_get_value(struct keyval *pairs, const char *key) {
	return kv_get_mut_value(pairs, key);
}

static struct keyval *kv_add_new_value(struct keyval *pairs, const char *key, const char *value) {
	struct keyval_pair_header *hdr = KV_GET_HEADER(pairs);
	size_t i;

	for (i = 0; pairs[i].value; i++)
		;

	if (i == hdr->capacity) {
		if (!(pairs = resize(pairs, hdr->capacity ? hdr->capacity * 2 : 8)))
			return NULL;
	}

	pairs[i].key = strdup(key);
	pairs[i].value = strdup(value);

	return pairs;
}

struct keyval *kv_set_value(struct keyval *pairs, const char *key, const char *value) {
	struct keyval *potential = kv_get_mut_pair(pairs, key);

	if (potential) {
		free(potential->value);
		potential->value = strdup(value);
	} else {
		pairs = kv_add_new_value(pairs, key, value);
	}

	return pairs;
}

struct keyval *kv_clear_key(struct keyval *pairs, const char *key) {
	struct keyval *kv = kv_get_mut_pair(pairs, key);

	if (kv) {
		free(kv->key);
		kv->key = NULL;
		free(kv->value);
		kv->value = NULL;
	}

	return pairs;
}

#if TEST == 1
#include <stdio.h>
#include <assert.h>

#define STR "Hello!"

int main() {
	struct keyval *kv = keyval_alloc();
	size_t i;

	kv = kv_set_value(kv, "hello", "world");
	kv = kv_set_value(kv, "goodbye", "world");

	assert(!strcmp(kv_get_value(kv, "hello"), "world") && "failed to get value for key hello");

	kv = kv_clear_key(kv, "goodbye");

	assert(kv_get_value(kv, "goodbye") == NULL);
	assert(kv_get_value(kv, "non-existent key") == NULL);

	/* this test was added to test actually reallocating a lot of times. */
	for (i = 0; i < 1000; i++) {
		char buffer[512];
		snprintf(buffer, sizeof(buffer), STR "%lu", i);

		kv = kv_set_value(kv, buffer, "statement");
	}

	assert(kv_get_value(kv, STR "123") != NULL);
	assert(!strcmp(kv_get_value(kv, STR "123"), "statement") && "stress test somehow failed");

	free_keyvals(kv);

	return 0;
}
#endif
