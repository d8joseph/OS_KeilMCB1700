/**
 * @file:   queue.c
 * @brief:  queue data structure
 * @author: pwrobel
 * @date:   2016/01/20
 */
#include "queue.h"
#include "k_rtx.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

void queue_init(QUEUE *q)
{
	int i;
	q->m_size = 0;
	q->m_start = 0;
	for (i = 0; i < MAX_QUEUE_SIZE; i++)
	{
		q->m_data[i] = NULL;
	}
}

void queue_add(QUEUE *q, void *data)
{
	if (q->m_size == MAX_QUEUE_SIZE) {
		#ifdef DEBUG_0
		  printf("Queue fail\n");
		#endif
		return;
	} else {
		q->m_data[(q->m_start + q->m_size) % MAX_QUEUE_SIZE] = data;
		q->m_size++;
	}
}

void queue_add_front(QUEUE *q, void *data) {
	if (q->m_size == MAX_QUEUE_SIZE) {
		#ifdef DEBUG_0
		  printf("Queue fail\n");
		#endif
		return;
	} else {
		q->m_start = (q->m_start - 1 + MAX_QUEUE_SIZE) % MAX_QUEUE_SIZE;
		q->m_data[q->m_start] = data;
		q->m_size++;
	}
}

void *queue_next(QUEUE *q)
{
	if (q->m_size == 0) {
		#ifdef DEBUG_0
		  printf("Queue fail\n");
		#endif
		return NULL;
	} else {
		void* data = q->m_data[q->m_start];
		q->m_size--;
		q->m_start++;
		if (q->m_start == MAX_QUEUE_SIZE) {
			q->m_start = 0;
		}
		return data;
	}
}

int queue_empty(QUEUE *q)
{
	return q->m_size == 0;
}

void queue_remove(QUEUE *q, void *data)
{
	int i;
	for (i = 0; i < q->m_size; i++) {
		if (q->m_data[(q->m_start + i) % MAX_QUEUE_SIZE] == data) {
			for (; i < q->m_size; i++) {
				q->m_data[(q->m_start + i) % MAX_QUEUE_SIZE] = q->m_data[(q->m_start + i + 1) % MAX_QUEUE_SIZE];

			}
			q->m_size -= 1;
			return;
		}
	}
}
