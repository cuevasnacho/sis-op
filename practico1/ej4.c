#include <unistd.h>
#include <stdio.h>

int main(void) {
	int x = 100;
	int rc = fork();
	if (rc == 0) {
		x = 50;
		printf("hijo: %d\n",x);
	} else if (rc > 0) {
		x = 75;
		printf("padre: %d\n",x);
	}
	return 0;
}
