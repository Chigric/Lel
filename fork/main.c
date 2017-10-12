#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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
			struct timespec mt1, mt2;
			long double t_1 = 0.0;
			clock_gettime(CLOCK_REALTIME, &mt1);
			waitpid(pid, status, 0);
			clock_gettime(CLOCK_REALTIME, &mt2);
			t_1 = 1000000000 * (mt2.tv_sec - mt1.tv_sec) + (mt2.tv_nsec - mt1.tv_nsec);
			t_1 = t_1/1000000;
			fprintf(datafile, "\n\nkek time = %Lf ms\n", t_1);
		}
	}
	fclose(datafile);
	return 0;
}
