#ifndef __IRDA_H__
#define __IRDA_H__

#include<pthread.h>

struct option_t{
    char* name;
    struct option_t* last;
    struct option_t* next;
    int opt;
};

struct irda_t{
    pthread_t tid;

    int t_cntl_fan;
    int pir_cntl_fan;
    int fan_level;
    struct option_t* option;

    pthread_mutex_t mutex;
    int power;

    int all_power;
};

void* irda_thread(void* arg);


#endif