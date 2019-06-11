//
//  main.c
//  MF_Test
//
//  Created by User on 5/22/19.
//  Copyright © 2019 TCrew. All rights reserved.
//

#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include "queue.h"

#define BANNER_DATA 24
#define IMG_SIZE 4
#define DEVICE_PORT 5546
#define CLIENT_PORT 7500

pthread_mutex_t mutex;

struct queue *queue;

struct sockaddr_in device_client_config;

int device_client_descriptor = 0;

unsigned int clients_connections_count = 0;

void write_to_file(int fprefix, char *fname, char *img, int size_img) {
    char fpath[30];
    sprintf(fpath, "%s%d%s%s", "/Users/user/", fprefix, fname, ".jpeg");
    printf("NAME === %s", fpath);
    
    FILE *f;
    f = fopen(fpath, "w");
    if(f != NULL) {
        for(int i=0; i < size_img; i++) {
            fputc(img[i], f);
        }
        fclose(f);
    } else {
        perror("error writing img data to file");
    }
}

struct sockaddr_in *sockaddr_in_init(struct sockaddr_in *sockaddr, uint16_t port, uint32_t s_addr) {
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(port);
    sockaddr->sin_addr.s_addr = htonl(s_addr);
    return 0;
}

void put_int_to_LE(char *buff, size_t buff_size, size_t number_to_devide) {
    if(buff == NULL) return;
    memset(buff, 0, buff_size);
    buff[0] = (char)(number_to_devide) >> 0;
    buff[1] = (char)(number_to_devide) >> 8;
    buff[2] = (char)(number_to_devide) >> 16;
    buff[3] = (char)(number_to_devide) >> 24;
}

void *handle_client(void *arg) {
    if(arg == NULL) {
        pthread_exit(0);
    }
    int *client_descriptor = NULL;
    client_descriptor = (int *)arg;
    
    unsigned int idx = 0;
    while(1) {
        struct timeval timeval;
        memset(&timeval, 0, sizeof(timeval));
        timeval.tv_sec = 0;
        timeval.tv_usec = 300000;
        fd_set readfd;

        FD_ZERO(&readfd);
        FD_SET(*client_descriptor, &readfd);
        if((select(*client_descriptor + 1, &readfd, NULL, NULL, &timeval) < 0) && (errno != EINTR)) {
            perror("SERVER: timeout for select\n");
            continue;
        }

        if(!FD_ISSET(*client_descriptor, &readfd)) {
            if(idx == queue->size) {
                continue;
            }

            if(queue->images[idx].size != 0 && queue->images[idx].buff != NULL) {
                char unrecovered_number[4];
                memset(unrecovered_number, 0, 4);

                unrecovered_number[0] = (char)(queue->images[idx].size >> 0);
                unrecovered_number[1] = (char)(queue->images[idx].size >> 8);
                unrecovered_number[2] = (char)(queue->images[idx].size >> 16);
                unrecovered_number[3] = (char)(queue->images[idx].size >> 24);
                
                printf("SERVER: CHECK number before and after convertion -> %d == %d\n", ((uint8_t)unrecovered_number[0] << 0) | ((uint8_t)unrecovered_number[1] << 8) | ((uint8_t)unrecovered_number[2] << 16) | ((uint8_t)unrecovered_number[3] << 24), queue->images[idx].size);
                
                if(send(*client_descriptor, (char *)unrecovered_number, 4, MSG_SEND) < 0) {
                    perror("SERVER: error sending image size\n");
                    break;
                }

                
                if(send(*client_descriptor, queue->images[idx].buff, queue->images[idx].size, MSG_SEND) < 0) {
                    perror("SERVER: error sending image data\n");
                    break;
                }

                if(idx < (queue->size - 1)) {
                    idx++;
                } else {
                    idx = queue->size - 1;
                }
            }
        }
    }

    pthread_mutex_trylock(&mutex);
    clients_connections_count--;
    pthread_mutex_unlock(&mutex);

    if(shutdown(*client_descriptor, SHUT_RD) != 0) pthread_exit(0);
    close(*client_descriptor);
    free(client_descriptor);

    pthread_exit(0);
}

void *start_fetching_images() {
    
    if(connect(device_client_descriptor, (const struct sockaddr *)&device_client_config, sizeof(device_client_config)) < 0) {
        perror("SERVER: error trying to connect device_client_descriptor\n");
        pthread_exit(0);
    }

    char banner_data[BANNER_DATA];
    memset(banner_data, 0, sizeof(banner_data));
    
    if(recv(device_client_descriptor, banner_data, sizeof(banner_data), 0) < 0) {
        perror("SERVER: error requesting banner data\n");
        pthread_exit(0);
    }

    while (clients_connections_count > 0) {
        char img_size_data[IMG_SIZE];
        memset(img_size_data, 0, IMG_SIZE);

        if(recv(device_client_descriptor, img_size_data, IMG_SIZE, 0) < 0) {
            perror("SERVER: error requesting image size\n");
            pthread_exit(0);
        }

        uint32_t img_size = 0;
        img_size = ((uint8_t)img_size_data[0] << 0) | ((uint8_t)img_size_data[1] << 8) | ((uint8_t)img_size_data[2] << 16) | ((uint8_t)img_size_data[3] << 24);
        
        char img_data[img_size];
        memset(img_data, 0, sizeof(img_data));
        size_t chunk_of_data = 0;
        size_t data_read = 0;

        while (data_read < img_size) {
            chunk_of_data = recv(device_client_descriptor, img_data + data_read, img_size - data_read, 0);
            if(chunk_of_data <= 0) {
                perror("SERVER: error requesting img_data\n");
                pthread_exit(0);
            }
            data_read = data_read + chunk_of_data;
            printf("SERVER: read %lu byte from %u byte, remain byte to read %lu byte\n", data_read, img_size, img_size - data_read);
        }

        struct image img;
        memset(&img, 0, sizeof(img));
        img.buff = img_data;
        img.size = img_size;
        push(&queue, img);
    }

    pthread_exit(0);
}

void *internal_serv_listen() {
    
    int intern_serv_descript = 0;
    intern_serv_descript = socket(AF_INET, SOCK_STREAM, 0);
    if(intern_serv_descript < 0) {
        perror("SERVER: error creating socket for internal server\n");
        pthread_exit(0);
    }

    if(setsockopt(intern_serv_descript, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("SERVER: error tryign to setsockopt\n");
    }

    struct sockaddr_in internal_serv_config;
    memset(&internal_serv_config, 0, sizeof(struct sockaddr_in));
    sockaddr_in_init(&internal_serv_config, CLIENT_PORT, INADDR_ANY);

    if(bind(intern_serv_descript, (const struct sockaddr *)&internal_serv_config, sizeof(internal_serv_config)) < 0) {
        perror("SERVER: error binding internal client server\n");
        pthread_exit(0);
    }

    if(listen(intern_serv_descript, 10) < 0) {
        perror("SERVER: error trying to invoke listent func\n");
        pthread_exit(0);
    }

    while(1) {
        int *client_descriptor = (int *)malloc(sizeof(int));
        printf("SERVER IS WAITING FOR CLIENT\n");
        *client_descriptor = accept(intern_serv_descript, NULL, NULL);
        if(*client_descriptor < 0) {
            perror("SERVER: error trying acceping client\n ");
            break;
        }
//        setsockopt(*client_descriptor, SOL_SOCKET, SO_NOSIGPIPE, &(int){1}, sizeof(int));
        printf("CLIENT CONNECTED TO SERVER\n");

        if(clients_connections_count == 0) {
            pthread_mutex_trylock(&mutex);
            clients_connections_count++;
            pthread_mutex_unlock(&mutex);

            pthread_t device_client_thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_create(&device_client_thread, &attr, start_fetching_images, NULL);
        } else {
            pthread_mutex_trylock(&mutex);
            clients_connections_count++;
            pthread_mutex_unlock(&mutex);
        }

        pthread_t client_thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&client_thread, &attr, handle_client, client_descriptor);
    }

    perror("SERVER DISCONECTED, DUE TO INVALID CLIETN ACCEPTION\n");
    pthread_exit(0);
}

int main(int argc, const char * argv[]) {
//    signal(SIGPIPE, SIG_IGN);
    device_client_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if(device_client_descriptor < 0) {
        perror("SERVER: error creating socket for device_client");
        return -1;
    }

    memset(&device_client_config, 0, sizeof(device_client_config));
    sockaddr_in_init(&device_client_config, DEVICE_PORT, INADDR_LOOPBACK);
    
    pthread_mutex_init(&mutex, NULL);

    queue = (struct queue*)queue_init();

    pthread_t device_thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&device_thread, &attr, internal_serv_listen, NULL);
    pthread_join(device_thread, NULL);

    return 0;
}

