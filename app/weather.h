#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <zlib.h>
#include <curl/curl.h>
#include <stdio.h>
#include <pthread.h>
#include <cJSON.h>

#define PATH_GZIP "/tmp/weather"

struct weather_t{
    pthread_t tid;
    FILE* zfp;
    gzFile gfp;
    CURL *curl;
    struct curl_slist *headers;
    char buf[512];
    char weather[8];
    // char temp[8];
    // char humidity[8];
    int temp;
    int humidity;

    int power;
    pthread_mutex_t mutex;
};

void *weather(void *arg);


#endif