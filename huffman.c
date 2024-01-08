#include "huffman.h"
#include "min_heap.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

TREE_NODE *treeNode_init(TREE_NODE *self, char data, bool isLeaf) {
  self->data = data;
  self->isLeaf = isLeaf;
  self->left = NULL;
  self->right = NULL;

  return self;
}

void treeNode_destroy(TREE_NODE *self) {
  self->data = 0;
  self->left = NULL;
  self->right = NULL;
}

void huffmanTree_destroy(TREE_NODE *root) {
  if (root->left != NULL) {
    huffmanTree_destroy(root->left);
  }

  if (root->right != NULL) {
    huffmanTree_destroy(root->right);
  }

  treeNode_destroy(root);
  free(root);
}

void calculateFrequencies(char *data, int dataSize, unsigned int *frequencies) {
  for (int i = 0; i < dataSize; i++) {
    ++frequencies[(unsigned char) data[i]];
  }
}

MIN_HEAP_ITEM *huffmanTree(MIN_HEAP *priorityQueue, unsigned int *frequencies) {
  // priority queue will be list of pointers to individual nodes
  for (int i = 0; i < TREE_NODE_COUNT; i++) {
    //create new tree node
    TREE_NODE *node = calloc(1, sizeof(TREE_NODE));
    treeNode_init(node, (char) i, true);
    // create new priority queue item
    MIN_HEAP_ITEM *item = calloc(1, sizeof(MIN_HEAP_ITEM));
    minHeapItem_init(item, node, frequencies[i]);
    //insert node
    minHeap_insert(priorityQueue, item);
  }

  while (priorityQueue->size != 1) {
    MIN_HEAP_ITEM *left = minHeap_get(priorityQueue);
    MIN_HEAP_ITEM *right = minHeap_get(priorityQueue);
    TREE_NODE *sumNode = calloc(1, sizeof(TREE_NODE));

    treeNode_init(sumNode, '$', false);
    // add left and right to the new node
    sumNode->left = left->data;
    sumNode->right = right->data;

    MIN_HEAP_ITEM *sumItem = calloc(1, sizeof(MIN_HEAP_ITEM));
    minHeapItem_init(sumItem, sumNode, left->priority + right->priority);
    free(left);
    free(right);

    minHeap_insert(priorityQueue, sumItem);
  }

  MIN_HEAP_ITEM *root = minHeap_get(priorityQueue);
  return root;
}

//traverses tree and sets resulting code to buffer
bool traverseHuffmanTree(TREE_NODE *root, unsigned int index, char *buf,
                         char c) {
  if (root->data == c && root->isLeaf) {
    buf[index] = '\0';
    return true;
  }

  bool result;
  if (root->left) {
    buf[index] = '0';
    result = traverseHuffmanTree(root->left, index + 1, buf, c);
    if (result) {
      return true;
    }
  }
  if (root->right) {
    buf[index] = '1';
    result = traverseHuffmanTree(root->right, index + 1, buf, c);
    if (result) {
      return true;
    }
  }

  return false;
}

//uses tree traversal to find encoded character
void getEncodedCharacter(TREE_NODE *root, char c, char *buf) {
  unsigned int index = 0;
  traverseHuffmanTree(root, index, buf, c);
}

void huffmanEncode(char *fileData, int fileSize, char *encoded,
                   int *encodedFileSize, unsigned int *frequencies) {
  calculateFrequencies(fileData, fileSize, frequencies);

  // huffman tree
  MIN_HEAP *priorityQueue = calloc(1, sizeof(MIN_HEAP));
  minHeap_init(priorityQueue, TREE_NODE_COUNT);
  MIN_HEAP_ITEM *rootItem = huffmanTree(priorityQueue, frequencies);
  TREE_NODE *rootNode = rootItem->data;

  //dump data
  int tmp[TREE_NODE_COUNT];
  int index = 0;
  //dumpHuffmanTree(rootNode, tmp, index);
  for (int i = 0; i < fileSize; i++) {
    char c = fileData[i];
    char encodedChar[TREE_NODE_COUNT + 1] = {0};
    encodedChar[TREE_NODE_COUNT] = '\0';
    getEncodedCharacter(rootNode, c, encodedChar);
    strcpy(encoded + index, encodedChar);
    index += strlen(encodedChar);
  }
  *encodedFileSize = index;

  // free the item, it wont be used
  free(rootItem);
  //destroy priority queue
  huffmanTree_destroy(rootNode);
  free(priorityQueue->array);
  free(priorityQueue);
}

void huffmanDecode(char *encoded, int encodedFileSize, char **decoded,
                   int *decodedFileSize, unsigned int *frequencies) {
  MIN_HEAP *priorityQueue = calloc(1, sizeof(MIN_HEAP));
  minHeap_init(priorityQueue, TREE_NODE_COUNT);
  MIN_HEAP_ITEM *rootItem = huffmanTree(priorityQueue, frequencies);
  TREE_NODE *node = rootItem->data;

  // assign memory, later realloc
  *decoded = calloc(10, sizeof(char));
  int allocatedSize = 10;

  int size = 0;
  char c;
  for (int i = 0; i < encodedFileSize; i++) {
    //while ((c = *encoded) != '\0') {
    c = encoded[i];
    if (c == '0') {
      node = node->left;
    } else {
      node = node->right;
    }

    // end of tree
    if (node->left == NULL && node->right == NULL) {
      (*decoded)[size] = node->data;
      node = rootItem->data;
      size++;

      // realloc
      if (size == allocatedSize) {
        allocatedSize *= 2;
        *decoded = realloc(*decoded, allocatedSize);
      }
    }
  }

  // free the item, it wont be used
  free(rootItem);
  huffmanTree_destroy(node);
  free(priorityQueue->array);
  free(priorityQueue);
}
