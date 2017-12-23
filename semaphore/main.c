#include <stdlib.h>
//EXIT_SUCCESS, rand()
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
#include <sys/sem.h>
//for semaphore..
#include <pwd.h>
//struct passwd

#define SIZE	 	128
#define QUAN_SEM	1
#define LOCK		-1
#define UNLOCK		1

//shared memory
void* shmlink;
int shmid;
bool rdon = true;//second process
//semaphore
int semid;

void signal_handler(int signal)
{
	if (signal == SIGINT) {	
		shmdt(shmlink);//attaches the shared memory
		if (!rdon){
			shmctl(shmid, IPC_RMID, NULL);//mark the segment to be destroyed
			int count = 0;
			for (; count < QUAN_SEM; ++count)
				semctl(semid, count, IPC_RMID);//delete all semaphores
		}
		exit(EXIT_SUCCESS);
	}
}

int main()
{
	setbuf(stdout, NULL);
	//magic. Now it is'n magic(.
	signal(SIGINT, signal_handler);

	int	id = 6;
	key_t key = ftok("../README.md", id);
	if (key == -1){
		perror("ftok");
		return 1;
	}
	printf("%x\n", (unsigned int)key);

	semid = semget(key, QUAN_SEM, IPC_CREAT | 0333);
	if (semid == -1) {
		perror("semget");
		return 2;
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
	int count;
	time_t realTime = NULL;
	//for semaphore
	struct sembuf s_buf;
	if (rdon) {
		printf("read:\n");
		for(;;){	
			for (count = 0; count < QUAN_SEM; ++count){
				s_buf.sem_num = count;
				s_buf.sem_op = LOCK;
				s_buf.sem_flg = SEM_UNDO;
				if (semop(semid, &s_buf, QUAN_SEM) == -1){
					perror("read_semop_LOCK");
					signal_handler(SIGINT);
				}

				printf("your time Mr.%s\t%s", pas->pw_name, (char*)shmlink);

				s_buf.sem_op = UNLOCK;
				if (semop(semid, &s_buf, QUAN_SEM) == -1){
					perror("read_semop_UNLOCK");
					signal_handler(SIGINT);
				}
			}
		}
	} else {
		printf("write:\n");
		//to initialize semaphores with UNLOCk
		for (count = 0; count < QUAN_SEM; ++count)
			semctl(semid, count, SETVAL, UNLOCK);
		for(;;){
			for (count = 0; count < QUAN_SEM; ++count){
				s_buf.sem_num = count;
				s_buf.sem_op = LOCK;
				s_buf.sem_flg = SEM_UNDO;
				if (semop(semid, &s_buf, QUAN_SEM) == -1){
					perror("write_semop_LOCK");
					signal_handler(SIGINT);
				}

				sleep(1); 
				time(&realTime);
				sprintf((char*)shmlink, "%s", ctime(&realTime));
	
				s_buf.sem_op = UNLOCK;
				if (semop(semid, &s_buf, QUAN_SEM) == -1){
					perror("write_semop_UNLOCK");
					signal_handler(SIGINT);
				}	
			}
		}
	}

	return 0;
}
