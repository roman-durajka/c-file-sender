#include "lzw.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

DICTIONARY_ITEM *dictionary_item_init(DICTIONARY_ITEM *self, char *data,
                                      size_t size) {
  self->data = data;
  self->size = size;

  return self;
}

DICTIONARY_ITEM *dictionary_item_destroy(DICTIONARY_ITEM *self) {
  free(self->data);
  self->data = NULL;
  self->size = 0;
}

DICTIONARY *dictionary_init(DICTIONARY *self) {
  self->size = 0;
  self->capacity = DICTIONARY_DEFAULT_CAPACITY;
  self->array = calloc(DICTIONARY_DEFAULT_CAPACITY, sizeof(DICTIONARY_ITEM *));

  return self;
}

void dictionary_destroy(DICTIONARY *self) {
  for (int i = 0; i < self->size; i++) {
    dictionary_item_destroy(self->array[i]);
    free(self->array[i]);
  }
  free(self->array);
  self->array = NULL;

  self->size = 0;
  self->capacity = 0;
}

void dictionary_insert(DICTIONARY *self, DICTIONARY_ITEM *item) {
  //ensure capacity
  if (self->size == self->capacity) {
    self->capacity += 10;
    self->array = realloc(self->array, self->capacity * sizeof(DICTIONARY_ITEM *));
  }

  //insert pointer
  self->array[self->size] = item;
  self->size++;
}

bool dictionary_contains(DICTIONARY *self, char *data, size_t size) {
  for (size_t i = 0; i < self->size; i++) {
    if (memcmp(self->array[i]->data, data,
               size <= self->array[i]->size ? size : self->array[i]->size) ==
        0 &&
        self->array[i]->size == size) {
      return true;
    }
  }
  return false;
}

size_t dictionary_getIndex(DICTIONARY *self, char *data, size_t size) {
  for (size_t i = 0; i < self->size; i++) {

    if (memcmp(self->array[i]->data, data,
               size <= self->array[i]->size ? size : self->array[i]->size) ==
        0 &&
        self->array[i]->size == size) {
      return i;
    }
  }

  return -1;
}

bool dictionary_hasKey(DICTIONARY *self, int code) {
  if (self->size > code) {
    return true;
  }
  return false;
}

DICTIONARY_ITEM *dictionary_get(DICTIONARY *self, size_t index) {
  return self->array[index];
}

void lzwEncode(char *fileData, int fileSize, int *encoded, int *encodedSize) {
  DICTIONARY dict;
  dictionary_init(&dict);
  // init dictionary with basic characters
  for (int i = -128; i < 128; i++) {
    // init data
    char *c = calloc(2, sizeof(char));
    c[0] = (char) i;
    c[1] = '\0';

    // init dict item
    DICTIONARY_ITEM *item = calloc(1, sizeof(DICTIONARY_ITEM));
    dictionary_item_init(item, c, 1);

    dictionary_insert(&dict, item);
  }

  int *result = calloc(fileSize, sizeof(int));
  int resultSize = 0;

  char currentString[256] = {0};
  int currentStringLen = 0;
  for (int i = 0; i < fileSize; i++) {
    char currentChar = fileData[i];
    char tmpString[256] = {0};
    memcpy(tmpString, currentString, currentStringLen);
    tmpString[currentStringLen] = currentChar;
    int tmpStringLen = currentStringLen + 1;

    // check if dict has tmp string
    bool contains = dictionary_contains(&dict, tmpString, tmpStringLen);
    if (contains) {
      // COPY TMP STRING TO CURRENT STRING
      //+ 1 to copy \0
      memcpy(currentString, tmpString, tmpStringLen + 1);
      currentStringLen = tmpStringLen;
      continue;
    }

    // add known string to output
    if (currentStringLen > 0) {
      int stringCode =
              dictionary_getIndex(&dict, currentString, currentStringLen);
      // or use result[resultSize] = stringCode;
      memcpy(result + resultSize, &stringCode, sizeof(int));
      resultSize++;
    }
    // add new string (known string + 1 new char) to dictionary
    char *newItem = calloc(tmpStringLen + 1, sizeof(char));
    memcpy(newItem, tmpString, tmpStringLen + 1);
    DICTIONARY_ITEM *newDictItem = calloc(1, sizeof(DICTIONARY_ITEM));
    dictionary_item_init(newDictItem, newItem, tmpStringLen);
    dictionary_insert(&dict, newDictItem);
    // reset known string and add new char to known string
    currentString[0] = currentChar;
    currentString[1] = '\0';
    currentStringLen = 1;
  }

  if (currentStringLen > 0) {
    int stringCode =
            dictionary_getIndex(&dict, currentString, currentStringLen);
    memcpy(result + resultSize, &stringCode, sizeof(int));
    resultSize++;
  }

  dictionary_destroy(&dict);

  // copy to output params
  memcpy(encoded, result, resultSize * sizeof(int));
  *encodedSize = resultSize * sizeof(int);

  free(result);
}

void lzwDecode(int *encoded, int encodedSize, char **decoded,
               int *decodedSize) {
  DICTIONARY dict;
  dictionary_init(&dict);
  // init dictionary with basic characters
  for (int i = -128; i < 128; i++) {
    // init data
    char *c = calloc(2, sizeof(char));
    c[0] = (char) i;
    c[1] = '\0';

    // init dict item
    DICTIONARY_ITEM *item = calloc(1, sizeof(DICTIONARY_ITEM));
    dictionary_item_init(item, c, 1);

    dictionary_insert(&dict, item);
  }

  char currentString[256] = {0};
  int currentStringLen = 0;
  // first character will always be part of the output
  DICTIONARY_ITEM *firstItem = dictionary_get(&dict, encoded[0]);
  *currentString = *firstItem->data;
  currentStringLen++;

  // init result
  char *result = calloc(encodedSize, sizeof(char));
  result[0] = *currentString;
  int resultCapacity = encodedSize;
  int resultSize = 1;

  for (int i = 1; i < encodedSize / sizeof(int); i++) {
    // get code
    int currentCode = encoded[i];

    // init tmp string and len
    char tmpString[256] = {0};
    int tmpStringLen = 0;

    if (dictionary_hasKey(&dict, currentCode)) {
      DICTIONARY_ITEM *item = dictionary_get(&dict, currentCode);

      memcpy(tmpString, item->data, item->size);
      tmpStringLen += item->size;
    } else {
      memcpy(tmpString, currentString, currentStringLen);
      tmpStringLen = currentStringLen;
      memcpy(tmpString + tmpStringLen, currentString, sizeof(char));
      tmpStringLen++;
    }

    // add to result
    // realloc if needed first
    if (resultSize + tmpStringLen > resultCapacity) {
      resultCapacity += 256;
      result = realloc(result, resultCapacity);
    }

    memcpy(result + resultSize, tmpString, tmpStringLen);
    resultSize += tmpStringLen;

    // add to dictionary
    char *newItemData = calloc(currentStringLen + 2, sizeof(char));
    memcpy(newItemData, currentString, currentStringLen);
    newItemData[currentStringLen] = tmpString[0];
    newItemData[currentStringLen + 1] = '\0';

    DICTIONARY_ITEM *newItem = calloc(1, sizeof(DICTIONARY_ITEM));
    dictionary_item_init(newItem, newItemData, currentStringLen + 1);

    dictionary_insert(&dict, newItem);

    // set tmp string to current string
    memcpy(currentString, tmpString, tmpStringLen + 1);
    currentStringLen = tmpStringLen;
  }
  // result[resultSize] = '\0';
  // resultSize++;
  *decoded = calloc(resultSize, sizeof(char));
  memcpy(*decoded, result, resultSize);
  *decodedSize = resultSize;

  dictionary_destroy(&dict);

  free(result);
}
