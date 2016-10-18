/**
 * @file:   min_heap.c
 * @brief:  min heap data structure
 * @author: pwrobel
 * @date:   2016/02/03
 */

#include "min_heap.h"
#include "k_rtx.h"

void bubble_up(MIN_HEAP *h, int i);
void bubble_down(MIN_HEAP *h);

void min_heap_init(MIN_HEAP *h, int (*pred)(void*, void*))
{
  h->m_size = 0;
  h->m_predicate = pred;
}

void min_heap_add(MIN_HEAP *h, void *data)
{
  if (h->m_size >= HEAP_MAX_SIZE) {
    return;
  }

  h->m_data[h->m_size] = data;
  h->m_size++;
  bubble_up(h, h->m_size - 1);
}
void* min_heap_next(MIN_HEAP *h)
{
  void* data;
  if (h->m_size <= 0) {
    return NULL;
  }
  data = h->m_data[0];
  h->m_data[0] = h->m_data[h->m_size - 1];
  h->m_size--;
  bubble_down(h);
  return data;
}
int min_heap_empty(MIN_HEAP *h)
{
  return (h->m_size <= 0);
}
void min_heap_remove(MIN_HEAP *h, void *data)
{
  int i;
  for (i = 0; i < h->m_size; i++) {
    if (h->m_data[i] == data) {
      while (i >= 1) {
        h->m_data[i] = h->m_data[(i - 1) / 2];
        i = (i - 1) / 2;
      }

      h->m_data[0] = h->m_data[h->m_size - 1];
      h->m_size--;
      bubble_down(h);

      break;
    }
  }
}

void bubble_up(MIN_HEAP *h, int i)
{
  while (i >= 1) {
    if (h->m_predicate(h->m_data[i], h->m_data[(i - 1) / 2])) {
      void* temp = h->m_data[i];
      h->m_data[i] = h->m_data[(i - 1) / 2];
      h->m_data[(i - 1) / 2] = temp;
      i = (i - 1) / 2;
    } else {
      break;
    }
  }
}
void bubble_down(MIN_HEAP *h)
{
  int current = 0;
  while (1) {
    // No child case
    if ((2 * current + 1) >= h->m_size) {
      return;
    }
    // One child case
    else if (2 * current + 2 >= h->m_size) {
      if (h->m_predicate(h->m_data[2 * current + 1], h->m_data[current])) {
        void* temp = h->m_data[current];
        h->m_data[current] = h->m_data[2 * current + 1];
        h->m_data[2 * current + 1] = temp;
        current = 2 * current + 1;
      } else {
        return;
      }
    }
    // Two children case
    else {
      int childIndex = h->m_predicate(h->m_data[2 * current + 1], h->m_data[2 * current + 2]) ?  2 * current + 1 : 2 * current + 2;

      if (h->m_predicate(h->m_data[childIndex], h->m_data[current])) {
        void* temp = h->m_data[current];
        h->m_data[current] = h->m_data[childIndex];
        h->m_data[childIndex] = temp;
        current = childIndex;
      } else {
        return;
      }
    }
  }
}
