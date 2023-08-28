#ifndef __PIR_H__
#define __PIR_H__

#include<pthread.h>

struct pir_t{
    pthread_t tid;
    int status;
    
    int power;
    pthread_mutex_t mutex;
};

void* pir_thread(void* arg);

#endif