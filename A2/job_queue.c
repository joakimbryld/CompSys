#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  // tjek om der er plads til det nedenstående.
  malloc(sizeof(void*[capacity]));
  void* arr[capacity];
  job_queue->array = arr;
  job_queue->isDestroyed = 0;
  job_queue->top = -1;

}

int job_queue_destroy(struct job_queue *job_queue) {

  
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  if (&job_queue->isDestroyed == 1) {
    printf("The job queue does not exist");
    return -1;
  }
  else { // skal tjekke om den er fuld
    job_queue->top = job_queue->top + 1;
    job_queue->array[job_queue->top] = data; 
  }
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  if (&job_queue->isDestroyed == 1) {
    printf("The job queue does not exist");
    return -1;
  }
  else {
    
    void** popped = job_queue->array[job_queue->top];
    
  } 
}
