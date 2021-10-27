#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int job_queue_init(struct job_queue *job_queue, int capacity) {

  void** arr = calloc(capacity, sizeof(void*));
  job_queue->array = arr;
  job_queue->isDestroyed = 0;
  job_queue->top = -1;
  job_queue->capacity = capacity;
  job_queue->NumbElements = 0;
  return 0;
}


int job_queue_destroy(struct job_queue *job_queue) {
  pthread_mutex_lock(&mutex);
  while (job_queue->NumbElements != 0) {
    pthread_cond_wait(&cond, &mutex);
  }

  if (job_queue->NumbElements == 0) { // skal gøre så den venter.
    job_queue->isDestroyed = 1;
    pthread_mutex_lock(&mutex); 
    return 0; 
  }
  return -1;
}




int job_queue_push(struct job_queue *job_queue, void *data) {
  printf("Før lås i push \n");
  pthread_mutex_lock(&mutex);
  printf("efter lås i push \n");
  if (job_queue->isDestroyed == 1) {
    printf("The job queue does not exist");
    pthread_mutex_unlock(&mutex);
    return -1;

  }
  while (job_queue->capacity == job_queue->NumbElements) { // filled
    printf("While loop \n ");
    pthread_cond_wait(&cond, &mutex);
    
  }

  

  if (job_queue->capacity > job_queue->NumbElements) { // not filled

    job_queue->top = job_queue->top + 1;
    job_queue->array[job_queue->top] = data;
    job_queue->NumbElements += 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 0;
     
  } 
  printf("Helt nede \n");
  return 0; // Ikke sikkert at det er rigtigt, bare en return værdi.
}




int job_queue_pop(struct job_queue *job_queue, void **data) {
  printf("Før lås i pop \n");
  pthread_mutex_lock(&mutex);
  printf("Efter lås i pop \n");
  if (job_queue->isDestroyed == 1) {
    printf("The job queue does not exist");
    pthread_mutex_unlock(&mutex);
    return -1;
  }

  while (job_queue->NumbElements == 0) { // empty
    printf("POP WHILE nede \n");
    pthread_cond_wait(&cond, &mutex);
  }

  data = malloc(sizeof(job_queue->array[job_queue->top]));
  job_queue->top = job_queue->top - 1;
  return 0;
  pthread_cond_signal(&cond); // skal give signal om at den har poppet et element.
  pthread_mutex_unlock(&mutex);

} 
