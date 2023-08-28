#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<linux/input.h>
#include <signal.h>
#include"pir.h"

struct input_event buf;

void* pir_thread(void* arg){
    struct pir_t* p = (struct pir_t*)arg;
    int fd = open("/dev/input/event2",O_RDWR);
    if(fd < 0){
        perror("open()");
        pthread_exit(NULL);
    }

    pthread_mutex_lock(&p->mutex);

    p->status = 0;
    pthread_mutex_unlock(&p->mutex);
    while(1){
        pthread_mutex_lock(&p->mutex);
        if(p->power == 0){
            pthread_mutex_unlock(&p->mutex);
            break;
        }

        pthread_mutex_unlock(&p->mutex);
        if(read(fd,&buf,sizeof(buf)) != sizeof(buf)){
            perror("pir_read()");
            continue;
        }
        pthread_mutex_lock(&p->mutex);
        if(buf.type != EV_KEY){
             pthread_mutex_unlock(&p->mutex);
             continue;
        }

        if(buf.code != KEY_5){
            printf("wrong_key\n");
            pthread_mutex_unlock(&p->mutex);
            continue;
        }

        if(buf.value == 1){
            p->status = 1;
     
        }
        else{
       
            p->status = 0;
        }
        pthread_kill(p->tid,SIGUSR1);
        pthread_mutex_unlock(&p->mutex);
    }

    close(fd); 
    pthread_exit(NULL);
}