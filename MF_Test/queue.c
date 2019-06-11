//
//  queue.c
//  Queue
//
//  Created by User on 5/17/19.
//  Copyright © 2019 TCrew. All rights reserved.
//

#include "queue.h"

unsigned int _size = 0;
unsigned int _idx = 0;

int is_full_q(void) {
    return _size == QUEUE_MAXCAPACITY;
}

int is_empty_q(void) {
    return _size == 0;
}

void push(struct queue **q, struct image elem) {
    struct queue *queue = *q;
    if(_size != 0) {
        _idx++;
        queue->rear++;
    }
    
    queue->images[_idx] = elem;
    
    _size++;
    queue->size = _size;
}

int pop_all_elems_from(struct queue **q) {
    (*q)->images = (struct image *)memset((*q)->images, 0, QUEUE_MAXCAPACITY * sizeof(struct image));
    (*q)->front = (struct image *)memset((*q)->front, 0, sizeof(struct image));
    (*q)->rear = (struct image *)memset((*q)->rear, 0, sizeof(struct image));
    (*q)->front = (*q)->images;
    (*q)->rear = (*q)->images;
    (*q)->size = 0;
    _size = 0;
    _idx = 0;
    return 0;
}

//struct image *pop(struct queue **queue) {
//    struct queue *q = *queue;
//
//    if(is_empty() != 0) {
//        perror("queue is already empty");
//        return q->front;
//    }
//    //free thre memory of first elem before pushing it
//    if(q->front->buff != NULL) {
//        free(q->front->buff);
//        q->front->size = 0;
//    }
//    struct image *image_to_pop = q->images;
//    free(image_to_pop);
//    struct image *create_new_image_and_pop_it = (struct image *)malloc(sizeof(struct image));
//    q->images[QUEUE_CAPACITY - 1] = *create_new_image_and_pop_it;
//
//    //change order of all elements. -1 step for all elems in queue
//    for(int i=0; i<_size; i++) {
//        if(i < _size - 1 || i == 0) {
//            q->images[i] = q->images[i+1];
//        }
//    }
//
//    //reset the front ptr so it will be enabled
//    q->front = q->images;
//
//    //Cleaning the memory
//    memset(q->images + _size - 1, 0, q->images[_size - 1].size);
//    q->images[_size - 1].size = 0;
//
//    if(*(q->rear) > 0) {
//        (*(q->rear))--;
//    }
//    if(_size > 0) {
//        _size--;
//    }
//    q->size = _size;
//    return q->front;
//}

void* queue_init() {
    struct queue *queue;
    queue = (struct queue *)malloc(sizeof(struct queue));
    
    queue->images = (struct image *)malloc(QUEUE_MAXCAPACITY * sizeof(struct image));
    queue->front = (struct image *)malloc(sizeof(struct image));
    queue->rear = (struct image *)malloc(sizeof(struct image));
    
    memset(queue->images, '0', sizeof(struct image));
    memset(queue->rear, '0', sizeof(struct image));
    memset(queue->front, '0', sizeof(struct image));
    
    //Check later. After this assigning memory for rear && fornt probably is lost!!!
    queue->rear = queue->images;
    queue->front = queue->images;
    queue->size = 0;
    
    return queue;
}



//Useless for now
/*
void* queue_init() {
    struct queue *queue;
    queue = (struct queue *)malloc(sizeof(struct queue));
    
    queue->array = (int *)malloc(QUEUE_CAPACITY * sizeof(int));
    queue->front = (int *)malloc(sizeof(int));
    queue->rear = (int *)malloc(sizeof(int));
    queue->front = queue->array;
    *queue->rear = 0;
    
    queue->isEmpty = ^{ return isEmpty(); };
    queue->isFull = ^{ return isFull(); };
    queue->deque = ^(struct queue **q) { return deque(q); };
    queue->enqueue = ^(struct queue **q, int elem) { return enqueue(q, elem); };
    queue->front_value = ^{ return front_value(queue->array); };
    queue->rear_value = ^{ return rear_value(queue->array, &(queue->rear)); };
    
    return queue;
}
 
 int front_value(int *array) {
 if(is_empty()) {
 perror("queue is empty");
 return -1;
 }
 return *array;
 }
 
 int rear_value(int *array, int** rear) {
 if(is_empty()) {
 perror("queue is empty");
 return -1;
 }
 return **rear;
 }

*/
