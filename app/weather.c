#include<stdlib.h>
#include<stdio.h>
#include <curl/curl.h>
#include<zlib.h>
#include<zconf.h>
#include<string.h>
#include<cJSON.h>
#include<pthread.h>
#include <unistd.h>
#include<signal.h>
#include "weather.h"
#include "temperature.h"

#define PATH_GZIP "/tmp/weather"

FILE* zfp;
gzFile gfp;
char wt[8];
char wh[8];

int create_weather(struct weather_t* w){

    w->curl = curl_easy_init();

    if(w->curl == NULL){
        perror("curl_easy_init()\n");
        return -1;
    }

    w->headers = NULL;
    w->headers = curl_slist_append(w->headers, "Accept: application/json");

    return 0;
}

void free_weather(struct weather_t* w){
    
    curl_slist_free_all(w->headers);
    curl_easy_cleanup(w->curl);
    // pthread_mutex_destroy(&w->mutex);
    // printf("free_weather()\n");
}

int parse_gz(struct weather_t* w){
    int ret;
    ret = gzread(gfp,w->buf,sizeof(w->buf));
    if(ret < 0){
        printf("gzread error:%d.\n",ret);
        return -1;
    }
    w->buf[ret] = '\0';
    cJSON* json = cJSON_Parse(w->buf);
    if(json == NULL){
        printf("cJSON_Parse() error\n");
        return -1;
    }
    cJSON* now = cJSON_GetObjectItem(json, "now");
    strcpy(w -> weather, cJSON_GetObjectItem(now, "text")->valuestring);
    strcpy(wt, cJSON_GetObjectItem(now, "temp")->valuestring);
    strcpy(wh, cJSON_GetObjectItem(now, "humidity")->valuestring);
    w -> temp = atoi(wt);
    w -> humidity = atoi(wh);
    cJSON_Delete(json);
    return 0;
}

int get_weather(struct weather_t* w){
    zfp = fopen(PATH_GZIP,"wb");
    curl_easy_setopt(w->curl, CURLOPT_HTTPHEADER, w->headers);// 改协议头
    curl_easy_setopt(w->curl, CURLOPT_URL,"https://devapi.qweather.com/v7/weather/now?location=118.72,32.24&key=38202bb9b0794938a606e8c51fb23440");
    curl_easy_setopt(w->curl, CURLOPT_WRITEDATA, zfp); //将返回的http头输出到fp指向的文件

    curl_easy_setopt(w->curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(w->curl, CURLOPT_SSL_VERIFYPEER, 0L);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    if(curl_easy_perform(w->curl) != 0){  // 执行
        perror("curl_easy_perform()\n");
        fclose(zfp);
        return -1;
    }
    fclose(zfp);
    gfp = gzopen(PATH_GZIP,"rb");
    if(parse_gz(w) < 0){
        printf("parse_gz() error\n");
        gzclose(gfp);
        return -1;
    }
    gzclose(gfp);
    return 0;
}



void *weather(void *arg){
    // printf("create weather\n");
    struct weather_t* weather = (struct weather_t*) arg;

    pthread_mutex_lock(&weather->mutex);

    if(create_weather(weather) < 0){
        printf("create_weather() error\n");
        pthread_mutex_unlock(&weather->mutex);
        pthread_exit(NULL);
    }

    pthread_mutex_unlock(&weather->mutex);
    while(1){
        pthread_mutex_lock(&weather->mutex);
        // printf("get weather\n");
        
        if(weather->power == 0){
            pthread_mutex_unlock(&weather->mutex);
            break;
        }

        if(get_weather(weather) < 0){
            printf("get_weather() error\n");
            pthread_mutex_unlock(&weather->mutex);
            continue;
        }
    
        pthread_kill(weather->tid,SIGUSR1);
    
        pthread_mutex_unlock(&weather->mutex);

        sleep(10);
    }
    free_weather(weather);
    pthread_exit(NULL);
}
