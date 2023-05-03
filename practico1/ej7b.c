#include <unistd.h>
#include <stdio.h>

int main(int argc, char ** argv) {
  if (argc<=1) 
    return 0;
  int rc = fork();
  if (rc<0)
    return -1;
  else if (0==rc)
    return 0;
  else {
    argv[argc-1] = NULL;
    execvp(argv[0], argv);
  }
}
