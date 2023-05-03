#include <unistd.h>
#include <stdio.h>

int main(void) {
	char * const args[] = {"/bin/date","-R",NULL};
	execv(args[0], args);
	printf("a\n");
	return 0;
}
