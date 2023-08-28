#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<pthread.h>
#include <signal.h>
#include<string.h>


#include"pir.h"
#include"irda.h"
#include"temperature.h"
#include"weather.h"

// #define gettid() syscall(__NR_gettid)

// #define IRDA_SIG SIGUSR1
// #define PIR_SIG SIGUSR2
// #define TEMP_SIG SIGINT
// #define WEATHER_SIG SIGALRM

typedef void (*sighandler_t)(int);

struct pir_t p;
struct irda_t i;
struct temperature_t t;
struct weather_t w;
pthread_t pthr,ithr,tthr,wthr;
int fan_fd;
char fan_lvl[1];
pthread_t tid;
sigset_t set;
int tag;


int init_struct(void){
    int err;
    fan_lvl[0] = '0';
    pthread_mutex_lock(&i.mutex);
    p.power = i.power;
    t.power = i.power;
    w.power = i.power;
    
    pthread_mutex_unlock(&i.mutex);
    p.tid = t.tid = w.tid = tid;
    p.status = 0;
    t.temp = 0;
    t.humidity = 0;
    t.fan_level = '0';
    w.weather[0] = '\0';
    w.temp = 0;  
    w.humidity = 0;

    err = pthread_create(&pthr,NULL,pir_thread,(void*)&p);

    if(err){
        perror("pthread_create(pir)");
        return -1;
    }
    err = pthread_create(&tthr,NULL,temperature,(void*)&t);

    if(err){
        perror("pthread_create(temperature)");
        pthread_mutex_lock(&p.mutex);
        p.power = 0;
        pthread_mutex_unlock(&p.mutex);
        pthread_join(pthr,NULL);
        return -1;
    }
    err = pthread_create(&wthr,NULL,weather,(void*)&w);

    if(err){
        perror("pthread_create(weather)");
        pthread_mutex_lock(&p.mutex);
        p.power = 0;
        pthread_mutex_unlock(&p.mutex);
        pthread_join(pthr,NULL);
        pthread_mutex_lock(&t.mutex);
        t.power = 0;
        pthread_mutex_unlock(&t.mutex);
        pthread_join(tthr,NULL);
        return -1;
    }
    return 0;
}

void destroy_struct(void){
    pthread_mutex_lock(&p.mutex);
    p.power = 0;
    pthread_mutex_unlock(&p.mutex);
    pthread_join(pthr,NULL);

    // pthread_mutex_destroy(&p.mutex);

    pthread_mutex_lock(&t.mutex);
    t.power = 0;
    pthread_mutex_unlock(&t.mutex);
    pthread_join(tthr,NULL);
  
    // pthread_mutex_destroy(&t.mutex);

    pthread_mutex_lock(&w.mutex);
    w.power = 0;
    pthread_mutex_unlock(&w.mutex);
    pthread_join(wthr,NULL);
  
    fan_lvl[0] = '0';
    write(fan_fd,fan_lvl,1);
    // pthread_mutex_destroy(&w.mutex);
}



void change_fan(void){
    pthread_mutex_lock(&i.mutex);
    if(i.pir_cntl_fan != 0){
        pthread_mutex_unlock(&i.mutex);
        pthread_mutex_lock(&p.mutex);
        if(p.status == 0){
            pthread_mutex_unlock(&p.mutex);
            write(fan_fd, "0", 1);
        }
        else{
            pthread_mutex_unlock(&p.mutex);
            pthread_mutex_lock(&i.mutex);
            if(i.t_cntl_fan != 0){
                pthread_mutex_unlock(&i.mutex);
                pthread_mutex_lock(&t.mutex);
                fan_lvl[0] = t.fan_level;
                write(fan_fd, fan_lvl, 1);
                pthread_mutex_unlock(&t.mutex);
            }
            else{
                fan_lvl[0] = i.fan_level + '0';
                write(fan_fd, fan_lvl, 1);
                pthread_mutex_unlock(&i.mutex);
            }
        }
    }
    else{
        if(i.t_cntl_fan != 0){
            pthread_mutex_unlock(&i.mutex);
            pthread_mutex_lock(&t.mutex);
            fan_lvl[0] = t.fan_level;
            write(fan_fd, fan_lvl, 1);
            pthread_mutex_unlock(&t.mutex);
        }
        else{
            fan_lvl[0] = i.fan_level + '0';
            write(fan_fd, fan_lvl, 1);
            pthread_mutex_unlock(&i.mutex);
        }
    }
}

void power_on(void){
    if(init_struct() < 0){
        printf("init_struct() failed\n");
        printf("please push back button\n");
        pthread_join(ithr,NULL);
        close(fan_fd);
        exit(1);
    }
}
 
// sighandler_t irda_handler(int signo){
//     pthread_mutex_lock(&i.mutex);
//     if(i.power != 0){
//         pthread_mutex_lock(&p.mutex);
//         if(p.power != 0){
//             pthread_mutex_unlock(&p.mutex);
//             if(i.pir_cntl_fan == 1){

//             }
//             else{

//             }
//         }
//         else{
//             pthread_mutex_unlock(&p.mutex);
//             power_on();
//             printf("Intelli_Fan start\n");
//             fflush(stdout);
//         }
        
//     }
//     else{
//         destroy_struct();
//         printf("Intelli_Fan stop\n");
//         fflush(stdout);
//     }
// }

// sighandler_t pir_handler(int signo){
    
// }

// sighandler_t temp_handler(int signo){
    
// }

sighandler_t handler(int signo){
    int a=1;
}


int main(void){
    fan_fd = open("/dev/fan",O_RDWR);

    if(fan_fd < 0){
        perror("open(fan)");
        exit(1);
    }

    tid = pthread_self();
    i.tid = tid;
    sigemptyset(&set);
    sigaddset(&set,SIGUSR1);
    // sigaddset(&set,PIR_SIG);
    // sigaddset(&set,TEMP_SIG);
    // sigaddset(&set,WEATHER_SIG);
    if(pthread_sigmask(SIG_UNBLOCK,&set,NULL)){
        perror("pthread_sigmask()");
        // write(fan_fd,'0',1);
        close(fan_fd);
        exit(1);
    }
    if(signal(SIGUSR1,handler) == SIG_ERR){
        perror("signal()");
        close(fan_fd);
        exit(1);
    }
    // if(signal(SIGUSR1,irda_handler) == SIG_ERR){
    //     perror("signal()");
    //     // write(fan_fd,'0',1);
    //     close(fan_fd);
    //     exit(1);
    // }
    // if(signal(PIR_SIG,pir_handler) == SIG_ERR){
    //     perror("signal(pir)");
    //     close(fan_fd);
    //     exit(1);
    // }
    // if(signal(TEMP_SIG,temp_handler) == SIG_ERR){
    //     perror("signal(temp)");
    //     close(fan_fd);
    //     exit(1);
    // }
    // if(signal(WEATHER_SIG,weather_handler) == SIG_ERR){
    //     perror("signal(weather)");
    //     close(fan_fd);
    //     exit(1);
    // }


    i.all_power = 1;
    if(pthread_create(&ithr,NULL,irda_thread,(void*)&i)){
        perror("pthread_create(irda)");
        close(fan_fd);
        exit(1);
    }
    char ww[8];

    int wt,wh,tt,th;
    tag = 0;
    int tagg = 0;
    int person;
    while(1){
     
        pause();
  
        pthread_mutex_lock(&i.mutex);
        
        if(i.all_power == 0){
            pthread_mutex_unlock(&i.mutex);
            break;
        }
        if(i.power == 0){
            pthread_mutex_unlock(&i.mutex);
     
            destroy_struct();
            tag = 1;
            printf("Intelli_Fan stop\n");
            fflush(stdout);
            continue;
        }
        pthread_mutex_unlock(&i.mutex);
        pthread_mutex_lock(&p.mutex);
        person = p.status;
       
        if(p.power == 0){
            pthread_mutex_unlock(&p.mutex);
            power_on();
            tag = 0;
            printf("Intelli_Fan start\n");
            fflush(stdout);
            continue;
        }
        pthread_mutex_unlock(&p.mutex);    
        change_fan();
        pthread_mutex_lock(&w.mutex);
        wt = w.temp;
        wh = w.humidity;
        strcpy(ww,w.weather);
        pthread_mutex_unlock(&w.mutex);
        pthread_mutex_lock(&t.mutex);
        tt = t.temp;
        th = t.humidity;
        pthread_mutex_unlock(&t.mutex);

        if(tagg == 0){
            tagg = 1;
        }
        else{
            tagg = 0;
            continue;
        }
        printf("______________________________________\n");
        printf("weather: %s\n",ww);
        printf("outdoor: %dC %dH\n",wt,wh);
        printf("indoor: %dC %dH\n",tt,th);
        printf("person: %d\n",person);
        
        fflush(stdout);
        if(wt < tt && tt > 25){
            printf("open window\n");
            printf("______________________________________\n\n\n");
            fflush(stdout);
            continue;
        }
        if(wt > tt && tt <20){
            printf("open window\n");
            printf("______________________________________\n\n\n");
            fflush(stdout);
            continue;
        }

        printf("close window\n");
        printf("______________________________________\n\n\n");
    }


    pthread_join(ithr,NULL);

    if(tag == 0)
        destroy_struct();
    close(fan_fd);
    exit(0);
}