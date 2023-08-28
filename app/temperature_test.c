#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include "temperature.h"

int main(void){
    struct temperature_t t;
    t.power = 1;
    t.temp = 0;
    t.humidity = 0;
    int err;
    pthread_t tthr;
    err = pthread_create(&tthr,NULL,temperature,(void*)&t);     
    if(err){
        perror("pthread_create()");
        exit(1);
    }
    sleep(1);
    pthread_mutex_lock(&t.mutex);
    t.power = 0;
    pthread_mutex_unlock(&t.mutex);
    // sleep(1);
    pthread_join(tthr,NULL); 
    pthread_mutex_destroy(&t.mutex);  
    printf("inside temp : %d\n", t.temp);
    printf("inside humidity : %d\n", t.humidity);
    exit(0);
}