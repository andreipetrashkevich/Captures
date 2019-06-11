//
//  queue.h
//  Queue
//
//  Created by User on 5/17/19.
//  Copyright © 2019 TCrew. All rights reserved.
//

#ifndef queue_h
#define queue_h

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "image.h"

#define QUEUE_MAXCAPACITY 100

//struct queue {
//    int *front;
//    int *rear;
//    int *array;
//
//    int(^isFull)(void);
//    int(^isEmpty)(void);
//    void(^enqueue)(struct queue **, int);
//    int(^deque)(struct queue **);
//    int(^front_value)(void);
//    int(^rear_value)(void);
//};

typedef void(^handle_client_callback)(struct image*);

struct queue {
    struct image *rear;
    struct image *front;
    struct image *images;
    unsigned int size;
};

void push(struct queue **q, struct image elem);

void* queue_init(void);

#endif /* queue_h */
