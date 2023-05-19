#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct
{
    size_t head;
    size_t tail;
    size_t capacity;
    size_t count;
    int** nodes;
} queue_t;

queue_t *create_queue(int capacity);

void delete_queue(queue_t *queue);

/*
 * inserts a reference to the element into the queue
 * returns: true on success; false otherwise
 */
bool push_to_queue(queue_t *queue, int *data);

/*
 * gets the first element from the queue and removes it from the queue
 * returns: the first element on success; NULL otherwise
 */
int *pop_from_queue(queue_t *queue);

/*
 * gets idx-th element from the queue, i.e., it returns the element that
 * will be popped after idx calls of the pop_from_queue()
 * returns: the idx-th element on success; NULL otherwise
 */
int *get_from_queue(queue_t *queue, int idx);

/* gets number of stored elements */
int get_queue_size(queue_t *queue);

/*check if number in queue*/
bool contains(queue_t *queue, int element);

#endif /* __QUEUE_H__ */
