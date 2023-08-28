#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include "irda.h"

int main(void){
    struct irda_t irda;
    pthread_t ithr;
    int err;
    pthread_mutex_unlock(&irda.mutex);
    err = pthread_create(&ithr,NULL,irda_thread,(void*)&irda);
    if(err){
        perror("pthread_create()");
        exit(1);
    }

    pthread_join(ithr,NULL);
    pthread_mutex_destroy(&irda.mutex); 
    exit(0) ;
}
