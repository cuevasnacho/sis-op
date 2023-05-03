#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int
str_to_int(char str[])
{
  int i = 0, sum = 0;

  while (str[i] != '\0') {
    if (str[i] < 48 || str[i] > 57) {	  // el 48 representa el '0' en ASCII
      printf("Unable to convert it to integer\n");
      return -1;
    } else {
      sum = sum * 10 + (str[i] - 48);
      ++i;
    }
  }

  return sum;
}

int
main(int argc, char *argv[])
{
  unsigned int id1 = 69;	              // creamos dos ids para los semaforos
  unsigned int id2 = 420;

  int n = 0;

  if (argc == 2) {		                  // (nombre del programa - cantidad de pingpongs)
    n = str_to_int(argv[1]);	          // transforma el string a un int
    if (n < 0) {		                    // si el argumento es menor que 0 no devuelve nada
      return -1;
    }
  } else {
    printf("Invalid amount of arguments, enter only one\n");	// si hay mas de un argumento, salta error
    return -1;
  }

  sem_open(id1, 1);		                  // abrimos los semaforos
  sem_open(id2, 0);

  int pid = fork();		                  // dividimos en dos procesos

  if (pid < 0) {
    printf("Error during the creation of child\n");
    sem_close(id1);
    sem_close(id2);
    return -1;
  }

  for (int m = 0; m < n; m++) {
    if (pid == 0) {
      sem_down(id1);
      printf("ping\n");
      printf("ping\n");
      sem_up(id2);
    } else {
      sem_down(id2);
      printf("\tpong\n");
      printf("\tpong\n");
      sem_up(id1);
    }
  }

  if (pid == 0) {
    sem_close(id1);
  } else {
    sem_close(id2);
  }

  return 0;
}
