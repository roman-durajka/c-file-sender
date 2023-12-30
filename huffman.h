#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "input_reader.h"
#include "min_heap.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TREE_NODE_COUNT 256

typedef struct tree_node {
    struct tree_node *left;
    struct tree_node *right;
    char data;
} TREE_NODE;

TREE_NODE *treeNode_init(TREE_NODE *self, char data);

void treeNode_destroy(TREE_NODE *self);

void huffmanTree_destroy(TREE_NODE *root);

void calculateFrequencies(char *data, int dataSize,
                          unsigned int *frequencies);

MIN_HEAP_ITEM *huffmanTree(MIN_HEAP *priorityQueue,
                           unsigned int *frequencies);

bool traverseHuffmanTree(TREE_NODE *root, unsigned int index,
                         char *buf, char c);

void getEncodedCharacter(TREE_NODE *root, char c, char *buf);

void huffmanEncode(char *fileData, int fileSize, char *encoded,
                   int *encodedFileSize, unsigned int *frequencies);

void huffmanDecode(char *encoded, int encodedFileSize, char **decoded,
                   int *decodedFileSize, unsigned int *frequencies);

#ifdef __cplusplus
}
#endif

#endif
