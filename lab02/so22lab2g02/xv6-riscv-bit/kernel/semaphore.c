#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "semaphore.h"
#include "defs.h"

#define true 1
#define false 0

struct spinlock general_lock;

struct semaphore sems[NSEM];

uint16 sem_are_set = false;
uint16 lock_set = false;

static void
set_sems(void)
{
  struct semaphore *aux_sem;

  for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) {
    aux_sem->value = -1;
  }
}

static void
decrease(struct semaphore *aux_sem)
{
  if (aux_sem->value == 0) {
    sleep(&aux_sem->sem_id, &aux_sem->lock);
    decrease(aux_sem);
  } else {
    aux_sem->value -= 1;
  }
}

int
sem_open(int sem, int value)
{

  if (value < 0) {					// el valor no puede ser negativo
    printf("Invalid value to set the semaphore\n");
    return 0;
  }

  struct semaphore *aux_sem;
  uint16 free_space = false;
  if (!lock_set) {					// se inicializa el lock por primera vez
    initlock(&general_lock, "general lock");
    lock_set = true;
  }
  acquire(&general_lock);

  if (!sem_are_set) {				// se inicializan los semaforos por primera vez
    set_sems();
    sem_are_set = true;
  }

  for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) {
    if (aux_sem->value == -1) {		// si el semaforo esta cerrado
      aux_sem->sem_id = sem;
      aux_sem->value = value;
      initlock(&aux_sem->lock, "sem lock");

      free_space = true;			// si hay algun semaforo libre
      break;
    }
  }

  if (!free_space) {				// siempre no hay semaforos disponibles
    printf("No free sems");
    release(&general_lock);
    return 0;						// devuelve 0 para decir que hubo un error
  }
  release(&general_lock);

  return 1;							// devuelve 1 para decir que todo salio bien
}

int
sem_close(int sem)
{

  struct semaphore *aux_sem;
  acquire(&general_lock);

  for (aux_sem = sems; aux_sem < &sems[NSEM] && (aux_sem->sem_id != sem);
       aux_sem++) ;
  // revisa todos los semaforos hasta encontrar primer el semaforo con misma id

  if (aux_sem->sem_id == sem && aux_sem->value != -1) {	// si matchea el id y esta abierto
    aux_sem->value = NPROC;			// setea el value en el maximo de procesos asi se asegura
    wakeup(&aux_sem->sem_id);		// que ninguno de los procesos que dormian en el sem se vuelvan a dormir
    aux_sem->value = -1;
  } else {
    printf("The sem doesn't exist\n");
    release(&general_lock);
    return 0;
  }

  release(&general_lock);

  return 1;
}

int
sem_up(int sem)
{

  struct semaphore *aux_sem;
  int exists_id = false;

  acquire(&general_lock);

  for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) {
    if ((aux_sem->sem_id) == sem) {	// busca el id
      if (aux_sem->value >= 0) {
	      exists_id = true;
	      break;							// una vez que encuentra el id se queda en el semaforo
      }
    }
  }

  if (!exists_id) {
    printf("That semaphore id doesn't exists\n");
    release(&general_lock);
    return 0;
  } else if (aux_sem->value == 0) {
    aux_sem->value += 1;
    wakeup(&aux_sem->sem_id);
  } else {
    aux_sem->value += 1;
  }

  release(&general_lock);

  return 1;
}

int
sem_down(int sem)
{

  struct semaphore *aux_sem;
  int exists_id = false;

  acquire(&general_lock);			// corta las interrupciones para asegurarse de que no
  									// se corte la ejecución de este proceso.

  for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) {
    if (aux_sem->sem_id == sem) {
      if (aux_sem->value >= 0) {
        exists_id = true;
        break;							// una vez que encuentra el id se queda en el semaforo
      }
    }
  }

  if (!exists_id) {
    printf("That semaphore id doesn't exists\n");
    release(&general_lock);
    return 0;
  } else {
    acquire(&aux_sem->lock);
    release(&general_lock);

    decrease(aux_sem);
    release(&aux_sem->lock);
  }

  return 1;
}
