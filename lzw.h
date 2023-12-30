#ifndef LZW_H
#define LZW_H

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DICTIONARY_DEFAULT_CAPACITY 266

typedef struct dictionary_item {
    char *data;
    size_t size;
} DICTIONARY_ITEM;

DICTIONARY_ITEM *dictionary_item_init(DICTIONARY_ITEM *self, char *data, size_t size);

DICTIONARY_ITEM *dictionary_item_destroy(DICTIONARY_ITEM *self);

typedef struct dictionary {
    DICTIONARY_ITEM **array;
    size_t size;
    size_t capacity;
} DICTIONARY;

DICTIONARY *dictionary_init(DICTIONARY *self);

void dictionary_destroy(DICTIONARY *self);

void dictionary_insert(DICTIONARY *self, DICTIONARY_ITEM *item);

bool dictionary_contains(DICTIONARY *self, char *data, size_t size);

size_t dictionary_getIndex(DICTIONARY *self, char *data, size_t size);

bool dictionary_hasKey(DICTIONARY *self, int code);

DICTIONARY_ITEM *dictionary_get(DICTIONARY *self, size_t index);

void lzwEncode(char *fileData, int fileSize, int *encoded, int *encodedSize);

void lzwDecode(int *encoded, int encodedSize, char **decoded, int *decodedSize);

#ifdef __cplusplus
}
#endif

#endif
