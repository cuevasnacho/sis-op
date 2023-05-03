#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  sem_open(2,0);
  sem_open(1,0);

  //int i = 0;
  
  // argv[1] numero de ping pongs

  int pid = fork();
  
  if (pid == 0) {
    printf("1\n");
    sem_up(1);
    sem_down(2);
    printf("3\n");
  } else {
    sem_down(1);
    printf("2\n");
    sem_up(2);
  }

  /*
  if (pid == 0) {
    sem_open(1,1);
    for (i=0; i<3; ++i) {
        printf("ping\n");
        sem_down(1);
        sem_up(2);
    }
    sem_close(1);
  } else {
    sem_open(2,0);
    for (i=0; i<3; ++i) {
        printf(" pong\n");
        sem_down(2);
        sem_up(1);
    }
    sem_close(2);
  }
  */


  exit(0);
}