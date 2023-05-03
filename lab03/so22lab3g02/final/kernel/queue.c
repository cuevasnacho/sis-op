#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"
#include "queue.h"

struct spinlock aux_lock;
int inited = 0;

// checks if a queue is empty
int queue_empty(queue q){
    return (q.size == 0);
}

// deletes the first element of the queue
struct proc *deque(queue *q){
    if (!inited){
        initlock(&aux_lock,"aux");
        inited =1;
    }
    acquire(&aux_lock);

    struct proc *p = 0;

    if (q->size > 0){
        p = q->elems[q->begin];
        q->begin = (q->begin +1) % NPROC;
        q->size--;
        release(&aux_lock);
        return p;
    } else{
        release(&aux_lock);
        panic("error: not processes in queue");
    }
}

// adds the element p to the queue
void enque(queue *q, struct proc *p){
    
    if (!inited){
        initlock(&aux_lock,"aux");
        inited =1;
    }
    acquire(&aux_lock);
    
    int position = 0;

    if (q->size < NPROC){
        position = (q->size + q->begin) % NPROC;
        q->size++;
    } else{
        release(&aux_lock);
        panic("error: process cannot be enqueued");
    }
    q->elems[position] = p;
    release(&aux_lock);
}