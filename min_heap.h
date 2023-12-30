#ifndef MIN_HEAP_H
#define MIN_HEAP_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct min_heap_item {
    long priority;
    void *data;
} MIN_HEAP_ITEM;

MIN_HEAP_ITEM *minHeapItem_init(MIN_HEAP_ITEM *self,
                                void *data, long priority);

typedef struct min_heap {
    size_t size;
    MIN_HEAP_ITEM **array;
} MIN_HEAP;

MIN_HEAP *minHeap_init(MIN_HEAP *self,
                       size_t capacity);

void minHeap_insert(MIN_HEAP *self,
                    MIN_HEAP_ITEM *item);

void minHeap_swap(MIN_HEAP_ITEM **item1,
                  MIN_HEAP_ITEM **item2);

void minHeap_rebuild(MIN_HEAP *self, size_t index);

MIN_HEAP_ITEM *minHeap_get(MIN_HEAP *self);

#ifdef __cplusplus
}
#endif

#endif

