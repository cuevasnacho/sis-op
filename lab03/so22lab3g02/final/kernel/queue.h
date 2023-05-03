#ifndef QUEUE_H
#define QUEUE_H

#include "param.h"
#include "proc.h"

typedef struct queue_s{
    struct proc* elems[NPROC];  // pointers to RUNNABLE processes
    int size;                   // number of processes
    int begin;                  // location on the array of the first enqueued process
} queue;

int queue_empty(queue q);

struct proc *deque(queue *q);

void enque(queue *q, struct proc *p);

#endif