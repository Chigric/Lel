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
//struct passwd

#define SIZE 128
	
void* shmlink;
int shmid;
bool rdon = true;//second process

void signal_handler(int signal)
{
	if (signal == SIGINT) {	
		shmdt(shmlink);//attaches the shared memory
		if (!rdon)
			shmctl(shmid, IPC_RMID, NULL);//mark the segment to be destroyed
		exit(EXIT_SUCCESS);
	}
}

int main()
{
	//magic. Now it is'n magic(.
	signal(SIGINT, signal_handler);

	int	id = 5;
	key_t key = ftok("main.c", id);
	if (key == -1){
		perror("ftok");
		return 1;
	}
	
	shmid = shmget(key, SIZE, 0);
	if (shmid == -1) {
		rdon = false;
		shmid = shmget(key, SIZE, IPC_CREAT | 0777);
		if (shmid == -1) {
			perror("shmget");
			return 1;
		}
	}

	shmlink = shmat(shmid, NULL, 0);
	if (shmlink == (void *) -1) {
		perror("shmat");
		return 1;
	}
	
	uid_t uid = getuid();
	struct passwd *pas = getpwuid(uid);
	time_t realTime = NULL;
	if (rdon) {
		while(1){
			sleep(1);
			printf("your time Mr.%s\t%s", pas->pw_name, (char*)shmlink);
		}
	} else {
		for(;;){
			sleep(1);	
			time(&realTime);
			sprintf((char*)shmlink, "%s", ctime(&realTime));
		}
	}

	return 0;
}
