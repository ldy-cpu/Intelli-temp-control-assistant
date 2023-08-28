#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<pthread.h>
#include <signal.h>
#include"temperature.h"

char lvl(int tt){
    if(tt >= 30)
        return '3';
    if(tt>=27)
        return '2';
    if(tt>=24)
        return '1';
    return '0';
}



void* temperature(void* arg){
    struct temperature_t* t = (struct temperature_t*)arg;
    int fd = open("/dev/dht11",O_RDWR);
    unsigned char buf[4];
    int l;
    if(fd < 0){
        perror("open()");
        pthread_exit(NULL);
    }
  
    while(1){
        pthread_mutex_lock(&t->mutex);
        if(t->power == 0){
            pthread_mutex_unlock(&t->mutex);
            break;
        }
        pthread_mutex_unlock(&t->mutex);
        if(read(fd,buf,4) < 0){
            perror("read()");
            continue;
        }
        pthread_mutex_lock(&t->mutex);     
        t->temp = (int)buf[2];
        t->humidity = (int)buf[0];
        l = lvl(t->temp);
        t->fan_level = l;
     
        pthread_kill(t->tid,SIGUSR1);

        pthread_mutex_unlock(&t->mutex);
        sleep(10);
    }

    close(fd);
    pthread_exit(NULL);
}