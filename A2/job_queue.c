#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  // Spørg TA om man skal tjekke om der er plads til det nedenstående.

  malloc(sizeof(void*[capacity]));
  void* arr[capacity];
  job_queue->array = arr;
  job_queue->isDestroyed = 0;
  job_queue->top = -1;
  job_queue->capacity = capacity;
  job_queue->NumbElements = 0;
  

}

int job_queue_destroy(struct job_queue *job_queue) {
  
  
}

// int checkIfArrayFull(struct job_queue *arr, int size) {
//   int counter = 0;
//   for (int i; i<size; i++){
//     if (arr->array[i] != NULL) {
//       counter++;
//     }
//   } 
//   if (counter == size) {
//     return 0;
//   }
//   else if (counter != size) {
//     return -1;
//   }
// }

int job_queue_push(struct job_queue *job_queue, void *data) {
  if (&job_queue->isDestroyed == 1) {
    printf("The job queue does not exist");
    return -1;
  }

  if (job_queue->capacity < job_queue->NumbElements) { // not filled
    job_queue->top = job_queue->top + 1;
    job_queue->array[job_queue->top] = data;
    job_queue->NumbElements += 1;
  } 

  else { // filled - needs to wait
  
  // use wait method.
  // mutex???

  }

}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  if (&job_queue->isDestroyed == 1) {
    printf("The job queue does not exist");
    return -1;
  }

  if (job_queue->NumbElements == 0) { // empty
    // wait for a push from another thread
  }

  else {
    malloc(sizeof(data));
    data = job_queue->array[job_queue->top];
    job_queue->top = job_queue->top - 1;

  } 
}
