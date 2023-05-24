#include "queue.h"

queue_t *create_queue(int capacity)
{
    queue_t *que = malloc(sizeof(queue_t));
    que->nodes = malloc(sizeof(int)*capacity);
    que->capacity = capacity;
    que->head= 0;
    que->tail= 0;
    que->count = 0;
    return que;
}

void delete_queue(queue_t *queue)
{
    free(queue->nodes);
    free(queue);
}

bool push_to_queue(queue_t *queue, int data)
{
    if(queue->head == queue->capacity){
        queue->capacity *= 2;
        int* tmp = realloc(queue->nodes, sizeof(int)*queue->capacity);
        if(tmp == NULL){
            exit(100);
        }
        queue->nodes = tmp;
    }
    queue->nodes[queue->head] = data;
    queue->head += 1;
    queue->count += 1;
    return true;
}

int pop_from_queue(queue_t *queue)
{
    if(queue->head == queue->tail){
        return -1;
    }
    
    if(queue->count < queue->capacity/3){
        for(size_t i = 0; i < queue->count; i++){
            queue->nodes[i] = queue->nodes[queue->tail+i];
        }
        int *p = realloc(queue->nodes, sizeof(int)*(queue->capacity/3));
        if(p != NULL){
            queue->nodes = p;
        }
        queue->capacity = queue->capacity/3;
        queue->tail = 0;
        queue->head = queue->count;
    }
    int tmp = queue->nodes[queue->tail];
    queue->tail += 1;
    queue->count -= 1;
    return tmp;
}

int get_from_queue(queue_t *queue, int idx)
{
    if(queue->count >= 1 && queue->tail+idx < queue->head){
        return queue->nodes[queue->tail+idx];
    }
    else{
        return -1;
    }
}

int get_queue_size(queue_t *queue)
{
    return queue->count;
}

bool contains(queue_t *queue, int element){
    for(int i = queue->tail; i < queue->head; i++){
        if(queue->nodes[i]==element){
            return true;
        }
    }
    return false;
}
