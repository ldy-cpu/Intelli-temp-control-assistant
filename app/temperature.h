#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

#include <pthread.h>

struct temperature_t {
    pthread_t tid;
    int temp;
    int humidity;
    char fan_level;

    int power;
    pthread_mutex_t mutex;
};

void *temperature(void* arg);

#endif