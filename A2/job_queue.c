#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "job_queue.h"


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER; 
pthread_cond_t fill  = PTHREAD_COND_INITIALIZER;

int job_queue_init(struct job_queue *jobqueue, int capacity) {
  jobqueue->max_Size = capacity;
  jobqueue->size = 0;
  jobqueue->head = 0;
  jobqueue->tail = -1;
  
  if( (jobqueue->elements = malloc(capacity * sizeof(void*))) != NULL) {
    return 0;
  }
  else {
    return 1;
  }
}

int job_queue_destroy(struct job_queue *jobqueue) {
  pthread_mutex_lock(&mutex);
  while(jobqueue->size != 0){ 
    pthread_cond_wait(&(empty),&mutex);
    }

  pthread_cond_broadcast(&(fill));
  pthread_mutex_unlock(&mutex);
  pthread_cond_destroy(&(fill));
  free(jobqueue->elements);
  free(jobqueue);
  return 0;
}

int job_queue_push(struct job_queue *jobqueue, void *data) {
  pthread_mutex_lock(&mutex);
  while(jobqueue->size == jobqueue->max_Size) { 
    pthread_cond_wait(&(empty), &mutex);
  }
  if(jobqueue!=NULL){
    if(jobqueue->tail == jobqueue->max_Size)
      jobqueue->tail = 0;
    else 
      jobqueue->tail++;
    jobqueue->elements[jobqueue->tail] = data;
    jobqueue->size++;

    pthread_cond_broadcast(&(fill));
    pthread_mutex_unlock(&mutex);
    return 1;
  }
  else
    return -1;
}

int job_queue_pop(struct job_queue *jobqueue, void **data) {
  pthread_mutex_lock(&mutex);
  while(jobqueue->size == 0)
    pthread_cond_wait(&(fill), &mutex);
  
  if(jobqueue!=NULL){
    *data =(char*) jobqueue->elements[jobqueue->head];
    //printf("%s popped from queue at head %d\n",  (char*)(jobqueue->elements[jobqueue->head]), jobqueue->head);
    jobqueue->size--;
    if(jobqueue->head == jobqueue->max_Size)
      jobqueue->head = 0;
    else
      jobqueue->head++;
    pthread_cond_signal(&(empty));
    pthread_mutex_unlock(&mutex);
    return 1;
  }
  else{
    data = NULL;
    return -1;
  }
}


