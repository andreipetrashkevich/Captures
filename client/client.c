//
//  main.c
//  client
//
//  Created by User on 5/11/19.
//  Copyright © 2019 TCrew. All rights reserved.
//

//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include <errno.h>
//#include <string.h>
//#include <sys/types.h>
//#include <time.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define PORT_OF_CLIENT 7500

int img_prefix = 0;

void write_to_file(int fprefix, char *fname, char *img, int size_img) {
    char fpath[30];
    sprintf(fpath, "%s%d%s%s", "/Users/user/", fprefix, fname, ".jpeg");
    printf("NAME === %s\n", fpath);
    
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

int main(int argc, const char * argv[]) {
    printf("Enter single char for global client name. Any character except 'enter'\n");
    char global_name[2] = {"n"};
    while (1) {
        char ch = fgetc(stdin);
        if(ch == '\n') break;
        *global_name = ch;
    }
    
    struct sockaddr_in client_sockaddr_config;
    int client_sock_descriptor;
    unsigned long rc;
    
    client_sockaddr_config.sin_family = AF_INET;
    client_sockaddr_config.sin_port = htons(PORT_OF_CLIENT);
    client_sockaddr_config.sin_addr.s_addr = htonl(INADDR_LOOPBACK);//inet_addr("127.0.0.1");
    
    client_sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock_descriptor < 0) {
        perror("CLIENT: Error while activaing a socket");
        return -1;
    }
    
    rc = connect(client_sock_descriptor, (struct sockaddr *)&client_sockaddr_config, sizeof(client_sockaddr_config));
    if (rc) {
        perror("CLIENT: Error invocking connect funcion");
        return -1;
    }
    printf("CLIENT IS CONNECTED TO SERVER AND WAITING\n");

    while (1) {
        char unrecoverd_number[4];
        memset(unrecoverd_number, 0, 4);

        rc = 0;
        rc = recv(client_sock_descriptor, unrecoverd_number, 4, 0);
        if(rc <= 0) {
            perror("CLIENT: error recv size img; SERVER FORCED TO DESTROY CONNECTION\n");
            break;
        }

        uint32_t img_size = 0;
        img_size = ((uint8_t)unrecoverd_number[0] | ((uint8_t)unrecoverd_number[1] << 8) | ((uint8_t)unrecoverd_number[2] << 16) | ((uint8_t)unrecoverd_number[3] << 24));

//        if((img_size = atoi(unrecoverd_number)) == 0) {
//            perror("CLIENT: error recovering img_size\n");
//            break;
//        }

        printf("CLIENT: got size of image %d\n", img_size);
        if(img_size == 1) {
            perror("CLIENT: error bad image size!!!\n");
            continue;
        }

        char *buff = NULL;
        buff = (char *)malloc(img_size);
        memset(buff, 0, img_size);
        size_t read = 0; rc = 0;

        while (read < img_size) {
            rc = recv(client_sock_descriptor, buff + read, img_size - read, 0);
            if(rc <= 0) {
                perror("CLIENT: error recv image itself\n");
                exit(-1);
            }
            read += rc;
        }
        printf("OK\n");

        
        write_to_file(img_prefix++, global_name, buff, img_size);
        free(buff);
    }
    printf("CLIENT IS DISCONNECTED\n");
    return 0;
}
