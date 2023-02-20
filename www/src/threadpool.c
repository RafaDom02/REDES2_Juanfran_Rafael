#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "threadpool.h"

void* thread_routine(void *arg){
    threadpool *tp;

    tp = (threadpool*) arg;

    while(1){
        //Lo que tenga que haccer el thread
    }
}

threadpool* threadpool_create(int num_threads){
    threadpool *tp;

    if(num_threads > NUM_MAX_THREADS || num_threads <= 0) return NULL;

    tp = (threadpool*)malloc(sizeof(threadpool));
    if(!tp) return tp;

    tp->threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
    if(!(tp->threads))
    {
        free(tp);
        return NULL;
    }

    tp->activated_threads = 0;
    tp->num_threads = num_threads;
    pthread_mutex_init(&(tp->mutex), NULL);

    return tp;
}

int threadpool_start(threadpool *tp){
    int i,j;

    for(i=0; i<tp->num_threads; i++)
    {
        if(pthread_create(&(tp->threads[i]), NULL, thread_routine, tp)) 
        {
            for(j=0; j<i; j++)
            {
                pthread_cancel(tp->threads[j]);
            }
            return -1;
        }
    }
    return 0;
}



void threadpool_destroy(threadpool *tp){
    
    pthread_mutex_destroy(&(tp->mutex));
    free(tp->threads);
    free(tp);
}