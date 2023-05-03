#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv) {
  if (0<--argc) {
    argv[argc] = NULL;
    execvp(argv[0],argv);
  } 

  return 0;
}
