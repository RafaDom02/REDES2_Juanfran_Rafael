#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include "threadpool.h"


threadpool *tpool;

void* thread_routine(void *arg){
    int i;
    int connfd = *((int*)arg);
    pthread_detach(pthread_self());
    pthread_mutex_lock(&(tpool->mutex));
    tpool->activated_threads++;
    pthread_mutex_unlock(&(tpool->mutex));

    //Lo que tenga que haccer el thread

    pthread_mutex_lock(&(tpool->mutex));
    tpool->activated_threads--;
    for(i=0; i<tpool->num_threads; i++){
        if(tpool->threads[i].fd == connfd){
            tpool->threads[i].fd == -1;
            break;
        }
    }
    pthread_mutex_unlock(&(tpool->mutex));
    pthread_exit(NULL);
}

threadpool* threadpool_create(int num_threads){
    threadpool *tp;
    int i;

    if(num_threads > NUM_MAX_THREADS || num_threads <= 0) return NULL;

    tp = (threadpool*)malloc(sizeof(threadpool));
    if(!tp) return tp;

    tp->threads = (serv_th*)malloc(num_threads*sizeof(serv_th));
    if(!(tp->threads))
    {
        free(tp);
        return NULL;
    }

    for(i=0; i<num_threads; i++) tp->threads[i].fd = -1;

    tp->activated_threads = 0;
    tp->num_threads = num_threads;
    pthread_mutex_init(&(tp->mutex), NULL);

    return tp;
}

int threadpool_start(threadpool *tp, int soc){
    int i,j, connfd;
    struct sockaddr_in cli;
    socklen_t len = sizeof(cli);
    tpool = tp;
    while(1){
        if(tpool->activated_threads != tpool->num_threads){
            connfd = accept(soc, (struct sockaddr*)&cli, &len);
            for(i=0; i<tpool->num_threads; i++){
                pthread_mutex_lock(&(tpool->mutex));
                if(tpool->threads[i].fd == -1){
                    tpool->threads[i].fd == connfd;
                    pthread_create(&(tpool->threads[i].thread), NULL, &thread_routine, (void*)&connfd);
                    break;
                }
                pthread_mutex_unlock(&(tpool->mutex));
            }
        }
    }
    return 0;
}



void threadpool_destroy(threadpool *tp){
    pthread_mutex_destroy(&(tp->mutex));
    free(tp->threads);
    free(tp);
}