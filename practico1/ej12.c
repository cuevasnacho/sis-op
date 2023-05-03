#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
  int pid = fork();
  if (pid == 0) {
    int c_pid = getppid();
    printf("%d %d\n",pid,c_pid);
  }
  wait(NULL);
  printf("%d",pid);

  return 0;
}
