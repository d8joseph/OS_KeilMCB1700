/**
 * @file:   queue.h
 * @brief:  queue data structure
 * @author: pwrobel
 * @date:   2016/01/20
 */
#ifndef QUEUE_H_
#define QUEUE_H_

#define MAX_QUEUE_SIZE 16

typedef struct queue {
	int m_start;
	int m_size;
	void *m_data[MAX_QUEUE_SIZE];
} QUEUE;

void queue_init(QUEUE *q);
void queue_add(QUEUE *q, void *data);
void queue_add_front(QUEUE *q, void *data);
void *queue_next(QUEUE *q);
int queue_empty(QUEUE *q);
void queue_remove(QUEUE *q, void *data);

#endif
