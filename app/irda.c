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
#include "irda.h"

int fd;
struct input_event buf;

int init_irda(struct irda_t* i){
    i->power = 0;
    i->t_cntl_fan = 0;
    i->pir_cntl_fan = 0;
    i->fan_level = 0;

    i->option = (struct option_t*)malloc(sizeof(struct option_t));
    if(i->option == NULL){
        perror("malloc()_1");
        return -1;
    }
    i->option->name = "temp_cntl_fan";
    i->option->opt = 0;

    i->option->next = (struct option_t*)malloc(sizeof(struct option_t));
    if(i->option->next == NULL){
        free(i->option);
        perror("malloc()_2");
        return -1;
    }
    i->option->next->last = i->option;
    i->option = i->option->next;
    i->option->name = "pir_cntl_fan";
    i->option->opt = 1;

    i->option->next = (struct option_t*)malloc(sizeof(struct option_t));
    if(i->option->next == NULL){
        free(i->option->last);
        free(i->option);
        perror("malloc()_3");
        return -1;
    }
    i->option->next->last = i->option;
    i->option = i->option->next;
    i->option->name = "fan_level";
    i->option->opt = 2;

    i->option->next = i->option->last->last;
    i->option->last->last->last = i->option;

    return 0;
}

void free_irda(struct irda_t* i){
    free(i->option->next->next);
    free(i->option->next);
    free(i->option);
}



void select_func(struct irda_t* i){
    switch (i->option->opt)
    {
    case 0:
        i->t_cntl_fan = !i->t_cntl_fan;
        break;
    case 1:
        i->pir_cntl_fan = !i->pir_cntl_fan;
        break;
    case 2:
        i->fan_level = (i->fan_level + 1) % 4;
        break;
    default:
        printf("wrong option\n");
        fflush(stdout);
        return;
    }
    pthread_kill(i->tid,SIGUSR1);
}


void print_option(struct irda_t* i){
    switch (i -> option -> opt)
    {
    case 0:
        printf("______________________\n%s      %d\n______________________\n\n\n",i->option->name,i->t_cntl_fan);    
        break;
    case 1:
        printf("______________________\n%s      %d\n______________________\n\n\n",i->option->name,i->pir_cntl_fan);
        break;
    case 2:
        printf("______________________\n%s      level:%d\n______________________\n\n\n",i->option->name,i->fan_level);
        break;
    default:
        printf("wrong option\n");
        break;
    } 
    fflush(stdout);
}

int get_button(struct irda_t* i){

    if(read(fd,&buf,sizeof(buf)) == sizeof(buf)){   
        if(buf.type != EV_KEY){
            return 0;
        }

        pthread_mutex_lock(&i->mutex);
        if(i->power == 0){
            switch (buf.code)
            {
            case 11:
                if(buf.value == 1){
                    printf("switch on\n");
                    fflush(stdout);
                    i->power = 1;
             
                    pthread_kill(i->tid,SIGUSR1);
           
                }
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }
                    
                break;
            
            case 2:
                if(buf.value == 1){
                    printf("back_all_shut\n");
                    fflush(stdout);
                    pthread_kill(i->tid,SIGUSR1);
                    pthread_mutex_unlock(&i->mutex);
                    return 999;
                }
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }

            default:
                pthread_mutex_unlock(&i->mutex);
                return -1;
            }
            print_option(i);
            pthread_mutex_unlock(&i->mutex);
            return 0;
        }

        switch(buf.code){
            case 11:
                
                if(buf.value == 1){
                    printf("switch off\n");
                    fflush(stdout);
                    i->power = 0;
                    pthread_kill(i->tid,SIGUSR1);
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }   
                break;
            case 2:
                if(buf.value == 1){
                    printf("back_all_shut\n");
                    fflush(stdout);
                    i->power = 0;
                    pthread_kill(i->tid,SIGUSR1);
                    pthread_mutex_unlock(&i->mutex);
                    return 999;
                }
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }

                    
                break;
            case 3:
                if(buf.value == 1){
                    // printf("up\n");
                    fflush(stdout);
                    i->option = i->option->last;
                }
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }
                break;
            case 4:
                if(buf.value == 1){
                    // printf("select\n");
                    fflush(stdout);
                    select_func(i);
                }
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }
                
                break;
            case 5:      
                if(buf.value == 1){
                    // printf("down\n");
                    fflush(stdout);
                    i->option = i->option->next;
                }
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }
                break;
            default:
                if(buf.value == 1)
                    pthread_mutex_unlock(&i->mutex);
                else{
                    pthread_mutex_unlock(&i->mutex);
                    return 0;
                }
                return -1;
        }
        pthread_mutex_unlock(&i->mutex);
    }
    else{
        printf("input_read error\n");
        fflush(stdout);
        return -1;
    }

    pthread_mutex_lock(&i->mutex);

    print_option(i);

    pthread_mutex_unlock(&i->mutex);
    return 0;
}




void* irda_thread(void* arg){
    struct irda_t* i = (struct irda_t*)arg;
    int o;
    fd = open("/dev/input/event1",O_RDWR);
    if(fd < 0){
        perror("open(event1)");
        pthread_mutex_lock(&i->mutex);
        i->all_power = 0;
        pthread_mutex_unlock(&i->mutex);
        pthread_exit(NULL);
    }
    pthread_mutex_lock(&i->mutex);
    if(init_irda(i) < 0){
        pthread_mutex_unlock(&i->mutex);
        printf("init_irda() error\n");
        fflush(stdout);
        close(fd);
        pthread_mutex_lock(&i->mutex);
        i->all_power = 0;
        pthread_mutex_unlock(&i->mutex);
        pthread_exit(NULL);
    }
    else
        pthread_mutex_unlock(&i->mutex);
    
    while(1){
        o = get_button(i);
        if( o < 0)
            printf("useless button\n");
        if(o == 999)
            break;
    }

    pthread_mutex_lock(&i->mutex);

    free_irda(i);

    close(fd);
    i->all_power = 0;
    pthread_mutex_unlock(&i->mutex);

    pthread_exit(NULL);
}