#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "types.h"
#include "param.h"
#include "spinlock.h"

struct semaphore {
  struct spinlock lock;
  unsigned int sem_id;
  int value;
};

int
sem_open(int sem, int value);
/* Initialize a semaphore sem with an 
 * arbitrary value 
 */

int
sem_close(int sem);
/* Closes a semaphore 
 */

int
sem_up(int sem);
/* Increases the semaphore's value 
 */

int
sem_down(int sem);
/* Decreases the semaphore's value
 */

#endif
