#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "job_queue.h"



pthread_cond_t go  = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int IsQdestroyed = 0;

pthread_cond_t clear = PTHREAD_COND_INITIALIZER; 

int job_queue_init(struct job_queue *jobqueue, int capacity) {
  jobqueue->cap = capacity;
  jobqueue->NumbOfElements = 0;
  jobqueue->top = 0;
  jobqueue->end = -1;
  
  if( (jobqueue->arr = malloc(capacity * sizeof(void*))) != NULL) {
    return 0;
  }
  else {
    return 1;
  }
}

int job_queue_destroy(struct job_queue *jobqueue) {
  pthread_mutex_lock(&mutex);
  while(jobqueue->NumbOfElements != 0){ 
    pthread_cond_wait(&(clear),&mutex);
    }
  pthread_cond_broadcast(&(go));
  pthread_mutex_unlock(&mutex);
  free(jobqueue->arr);
  IsQdestroyed=1;
  return 0;
}

int job_queue_push(struct job_queue *jobqueue, void *data) {
  pthread_mutex_lock(&mutex);
  while(jobqueue->NumbOfElements == jobqueue->cap) { 
    pthread_cond_wait(&(clear), &mutex);
  }
  if(jobqueue!=NULL){
    if(jobqueue->end == jobqueue->cap)
      jobqueue->end = 0;
    else 
      jobqueue->end = jobqueue->end +1;
    jobqueue->arr[jobqueue->end] = data;
    jobqueue->NumbOfElements = jobqueue->NumbOfElements + 1;
    
    pthread_cond_broadcast(&(go));
    pthread_mutex_unlock(&mutex);
    return 1;
  }
  else
    return -1;
}

int job_queue_pop(struct job_queue *jobqueue, void **data) {
  int c = 0;
  pthread_mutex_lock(&mutex);

  while((jobqueue->NumbOfElements == 0)&&(!IsQdestroyed))
    pthread_cond_wait(&(go), &mutex);
  
  if(!IsQdestroyed){
    *data =(char*) jobqueue->arr[jobqueue->top];

    jobqueue->NumbOfElements = jobqueue->NumbOfElements -1 ;
    if(jobqueue->top == jobqueue->cap)
      jobqueue->top = 0;
    else
      jobqueue->top = jobqueue->top + 1;
    pthread_cond_signal(&(clear));
  }
  else{
    c = -1;
    data = NULL;
  }
  pthread_mutex_unlock(&mutex);
  return c;
}
