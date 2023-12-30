#include "min_heap.h"

#include <stdlib.h>

MIN_HEAP_ITEM *minHeapItem_init(MIN_HEAP_ITEM *self,
                                void *data, long priority) {
  self->priority = priority;
  self->data = data;

  return self;
}

MIN_HEAP *minHeap_init(MIN_HEAP *self, size_t capacity) {
  self->size = 0;
  self->array = calloc(capacity, sizeof(MIN_HEAP_ITEM *));

  return self;
}

void minHeap_insert(MIN_HEAP *self, MIN_HEAP_ITEM *item) {
  size_t index = self->size;
  self->size++;

  while (index > 0 && item->priority < self->array[(index - 1) / 2]->priority) {
    self->array[index] = self->array[(index - 1) / 2];
    index = (index - 1) / 2;
  }

  self->array[index] = item;
}

void minHeap_swap(MIN_HEAP_ITEM **item1,
                  MIN_HEAP_ITEM **item2) {
  MIN_HEAP_ITEM *tmp = *item1;
  *item1 = *item2;
  *item2 = tmp;
}

void minHeap_rebuild(MIN_HEAP *self, size_t index) {
  size_t current = index;
  size_t left = 2 * current + 1;
  size_t right = 2 * current + 2;

  if (left < self->size &&
      self->array[left]->priority < self->array[current]->priority)
    current = left;

  if (right < self->size &&
      self->array[right]->priority < self->array[current]->priority)
    current = right;

  if (current != index) {
    minHeap_swap(&self->array[index], &self->array[current]);
    minHeap_rebuild(self, current);
  }
}

MIN_HEAP_ITEM *minHeap_get(MIN_HEAP *self) {
  MIN_HEAP_ITEM *item = self->array[0];
  self->array[0] = self->array[self->size - 1];
  self->size--;

  //rebuild
  minHeap_rebuild(self, 0);

  return item;
}