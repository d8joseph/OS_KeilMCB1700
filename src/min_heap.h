/**
 * @file:   min_heap.h
 * @brief:  min heap data structure
 * @author: pwrobel
 * @date:   2016/02/03
 */
#ifndef MIN_HEAP_H_
#define MIN_HEAP_H_

#define HEAP_MAX_SIZE 16

typedef struct min_heap {
  int m_size;
  void *m_data[HEAP_MAX_SIZE];
  int (*m_predicate)(void*, void*);
} MIN_HEAP;

void min_heap_init(MIN_HEAP *h, int (*pred)(void*, void*));
void min_heap_add(MIN_HEAP *h, void *data);
void* min_heap_next(MIN_HEAP *h);
int min_heap_empty(MIN_HEAP *h);
void min_heap_remove(MIN_HEAP *h, void *data);

#endif
