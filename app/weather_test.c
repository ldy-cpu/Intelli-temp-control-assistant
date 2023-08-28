#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include "weather.h"

int main(void){
    struct weather_t w;
    w.weather[0] = '\0';
    w.temp[0] = '\0';
    w.humidity[0] = '\0';
    w.power = 1;
    int err;
    pthread_t wthr;
    err = pthread_create(&wthr,NULL,weather,(void*)&w);     
    if(err){
        perror("pthread_create()");
        exit(1);
    }
    sleep(1);
    pthread_mutex_lock(&w.mutex);
    w.power = 0;
    pthread_mutex_unlock(&w.mutex);
    // sleep(1);
    pthread_join(wthr,NULL); 
    pthread_mutex_destroy(&w.mutex);  
    printf("weather : %s\n", w.weather);
    printf("temp : %s\n", w.temp);
    printf("humidity : %s\n", w.humidity);
    exit(0);
}