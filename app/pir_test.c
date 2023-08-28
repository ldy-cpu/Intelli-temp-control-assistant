#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include "pir.h"

int main(void){
    struct pir_t pir;
    pthread_t pthr;
    int err;
    pir.power = 1;
    pthread_mutex_unlock(&pir.mutex);
    err = pthread_create(&pthr,NULL,pir_thread,(void*)&pir);
    if(err){
        perror("pthread_create()");
        exit(1);
    }
    sleep(50);
    pthread_mutex_lock(&pir.mutex);
    pir.power = 0;
    pthread_mutex_unlock(&pir.mutex);

    pthread_join(pthr,NULL);
    pthread_mutex_destroy(&pir.mutex); 
    exit(0) ;
}