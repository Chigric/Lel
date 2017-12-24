#include <stdlib.h>
//EXIT_SUCCESS
#include <stdio.h>
//perror()
#include <time.h>
//time()
#include <sys/ipc.h> 
//ftok()
#include <sys/shm.h>
//shmget(), shmctl(), shmat(), shmdt()
#include <signal.h>
//signal(s)...
#include <string.h>
//memset()
#include <stdbool.h>
//bool
#include <unistd.h>
//sleep()
#include <pwd.h>
//struct passwdinclude <stdio.h> 
#include <fcntl.h>
//H_TH
#include <sys/stat.h>
//stat
#include <pthread.h>
//fifo...

#define SIZE 32

char* fifo_path = "I_did_not_hit_her_It’s_not_true_It’s_bullshit_I_did_not_hit_her._I_did_not_Oh_hi_Mark";
bool rdon = true;
int fifo_fd;
char buf[SIZE];

void signal_handler(int signal){
	if(signal == SIGINT){
		if(!rdon)
	    		unlink(fifo_path);
		exit(EXIT_SUCCESS);
	}
	if(signal == SIGPIPE){
		unlink(fifo_path);
		exit(EXIT_SUCCESS);
    	}
    	printf("signal %d", signal);
}

int main(){
	setbuf(stdout, NULL);
    	signal(SIGINT, signal_handler);
    	signal(SIGPIPE, signal_handler);

    	struct stat st;
    	if(stat(fifo_path, &st) != 0){
	    	rdon = false;
		printf("fifo is'n exist, trying to create\n");
		if (mkfifo(fifo_path, 0776)) {
			perror("mkfifo");
			exit(EXIT_FAILURE);
		}
    	}

	uid_t uid = getuid();
	struct passwd *pas = getpwuid(uid);
	time_t realTime = NULL;
	if(rdon){
	        printf("read\n");
		fifo_fd = open(fifo_path, O_RDONLY);
		if (fifo_fd == -1) {
			perror("read_fifo_open");
			exit(EXIT_FAILURE);
		}
		for(;;){
		    	sleep(1);        
	    		read(fifo_fd, &buf, sizeof(char)*SIZE);
	    		printf("your time Mr.%s\t%s", pas->pw_name, buf);
		}
    	} else {
		printf("write\n");
		fifo_fd = open(fifo_path, O_WRONLY);
		if (fifo_fd == -1) {
			perror("write_fifo_open");
			exit(EXIT_FAILURE);
		}
		for(;;){
		    	sleep(1);
	    		time(&realTime);
	    		printf("%s\n", ctime(&realTime));
	    		dprintf(fifo_fd, "%s\n", ctime(&realTime));
		}
    	}
}   
