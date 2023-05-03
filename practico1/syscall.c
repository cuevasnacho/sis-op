#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
	char * argv[] = {"pi","100000",NULL};
	int fp = fork();
	if (fp == 0) {
		execvp("pi",argv);
	} else {
		printf("soy el padre");
	}
	return 0;
}
