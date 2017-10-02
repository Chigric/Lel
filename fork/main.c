#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
int main()
{
	pid_t pid;
	FILE *datafile = fopen("data.txt", "wt");
	if(datafile == NULL){
		perror("FAIL WITH FAIL!");
		return 1;
	}
	switch (pid = fork()) {
		case -1: {
			perror("fork");
			exit(2);
		}			
		case 0:{
			fprintf(datafile, "It's child PID = %d\n", getpid());
			fprintf(datafile, "It's parent PID = %d\n\n", getppid());
			register int i;
			unsigned int sum = 1024 * 1024 * 10;
			for (i = 0; i < sum; i++)
				fprintf(datafile, "1");
//			sleep(2);
			exit(0);
		}
		default: {
			int *status = NULL; 
			time_t time_1 = time(NULL);
			waitpid(pid, status, 0);
			time_1 = time(NULL) - time_1;
			fprintf(datafile, "\n\nkek time = %f\n", (double)time_1);
		}
	}
	fclose(datafile);
	return 0;
}
