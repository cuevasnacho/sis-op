#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "syscall.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"
#include "semaphore.h"

#define true 1
#define false 0

struct spinlock general_lock;

struct semaphore sems[NSEM];

uint16 sem_are_set = false;
uint16 lock_set = false;

static void set_sems(void) {
	struct semaphore * aux_sem;

	for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) {
		aux_sem->value = -1;
	}
}

/* Initialize a semaphore sem with an 
 * arbitrary value 
 */
int
sem_open(int sem, int value) {

	struct semaphore * aux_sem;
	uint16 free_space = false;
	if (!lock_set){
		initlock(&general_lock,"general lock");
		lock_set = true;
	}
	acquire(&general_lock);
	

	if (!sem_are_set) {
		set_sems();
		sem_are_set = true;
	}
	/*usar la idea de next_sem(proximo semaforo despues del seteado)*/ 

	for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) {
		if (aux_sem->value == -1) {
			aux_sem->sem_id = sem;
			aux_sem->value = value;
			initlock(&aux_sem->lock, "sem lock");
			free_space = true;
			break;
		}
	}

	if(!free_space){
		printf("No free sems");
		return 0;		// devuelve 0 para decir que hubo un error
	}

	release(&general_lock);
	
	return 1; 				// devuelve 1 para decir que todo salio bien
}

int 
sem_close(int sem){

	struct semaphore * aux_sem;
	acquire(&general_lock);

	for (aux_sem = sems; aux_sem < &sems[NSEM] && (aux_sem->sem_id != sem); aux_sem++); // 
	// revisa todos los semaforos hasta encontrar primer el semaforo con misma id 
 

	if (aux_sem->sem_id == sem && aux_sem->value != -1){
		
		aux_sem->value = NPROC; // setea el value en el maximo de procesos asi se asegura
		wakeup(&aux_sem);		// que ninguno de los procesos que dormian en el sem se vuelvan a dormir
		aux_sem->value = -1;
	}else{
		release(&general_lock);
		printf("the sem doesn't exist or is already close \n");
		return 0;
	}
	


	release(&general_lock);

	return 1;
}

int
sem_up(int sem) {

	struct semaphore * aux_sem;
	int exists_id = false;

	acquire(&general_lock);
	/* deberiamos lockear el proceso con acquire
	 * ver con un ciclo que semaforo(aux_sem) tienes al id sem,
	 * que se quede apuntando a ese,
	 * si el aux_sem->value = 0 despierta al proceso wake up,
	 * incrementa normalmente con aux_sem->value += 1
	 * release el proceso para unlockear
	 */

	for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) {
		if (aux_sem->sem_id == sem) {
			if(aux_sem->value >= 0 ){
				exists_id = true;
				break;		// una vez que encuentra el id se queda en el semaforo
			}
		}
	}


	if (!exists_id) {
		printf("That semaphore id doesn't exists");
		return 0;
	} else if (aux_sem->value == 0) {
		aux_sem->value += 1;
		wakeup(&aux_sem);
	} else {
		aux_sem->value += 1;
	}

	printf("up: %d %d\n",aux_sem->value,aux_sem->sem_id);

	release(&general_lock);
	
	return 1;
}
/* Increases the semaphore's value 
 */
static void decrease(struct semaphore * aux_sem){
	if (aux_sem->value == 0) {
		sleep(&aux_sem, &aux_sem->lock);

		decrease(aux_sem);
	} else {
		aux_sem->value -= 1;
	}
}
	

int
sem_down(int sem) {

	struct semaphore * aux_sem;
	int exists_id = false;

	acquire(&general_lock);		//corta las interrupciones para asegurarse de que no se corte la ejecución de
								//este proceso.


	for (aux_sem = sems; aux_sem < &sems[NSEM]; aux_sem++) { //no deberia sumar el tamaño del semáforo?
		if (aux_sem->sem_id == sem) {
			if(aux_sem->value >= 0 ){
				exists_id = true;
				break;		// una vez que encuentra el id se queda en el semaforo
			}	
		}			
	}


	if (!exists_id) {
		release(&general_lock);
		printf("That semaphore id doesn't exists");
		return 0;
	}else {
		acquire(&aux_sem->lock);
		release(&general_lock);
	
		decrease(aux_sem);
		release(&aux_sem->lock);
	}

	printf("down: %d %d\n",aux_sem->value,aux_sem->sem_id);

	return 1;
/* Decrease the semaphore's value 
 */
	/* ver con un ciclo que semaforo(aux_sem) tienes al id sem,
	 * que se quede apuntando a ese, si el aux_sem->value = 0 duerme al proceso sleep,
	 * si no lo es, decrementa el aux_sem->value -= 1
	 */

}
/* Decreases the semaphore's value */
