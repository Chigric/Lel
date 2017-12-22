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
#include <pthread.h>
//pthread and mutex

#define SIZE 	128
#define QUAN_TH 5//quantity threads

//shared memory
void* shmlink;
int shmid;
bool rdon = true;//second process

//pthread
pthread_mutex_t mutex;
pthread_t pth[QUAN_TH];

void signal_handler(int signal)
{
	if (signal == SIGINT) {	
		shmdt(shmlink);//attaches the shared memory
		if (!rdon)
			shmctl(shmid, IPC_RMID, NULL);//mark the segment to be destroyed
		exit(EXIT_SUCCESS);
	}
}

void* read_th(void* c_)
{//read from shared memory and print to term. 
	while(true) {	
		sleep(1);
		pthread_mutex_lock(&mutex);
		printf("%x - %s", (unsigned int)pthread_self(), (char*)shmlink);
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

void* write_th(void* c_)
{//wtire to shared memory
	while(true) {
		sleep(1);
		pthread_mutex_lock(&mutex);
		time_t realTime = NULL;
		time(&realTime);
		sprintf((char*)shmlink, "%s", ctime(&realTime));
		pthread_mutex_unlock(&mutex);
	}

	return NULL;
}

int main()
{
	printf("TREAD_ID - real_Time\n");
	//magic. Now it is'n magic(.
	signal(SIGINT, signal_handler);

	int status;//check
	status = pthread_mutex_init(&mutex, NULL);
	if (status != 0) {
		perror("pthread_mutex_init");
		return 2;
	}

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
	
	int count;
	if (rdon) {
		for(count = 0; count < QUAN_TH; count++){
			pthread_create(&(pth[count]), NULL, &read_th, NULL);//thread for read
		}
	} else {
		for(count = 0; count < QUAN_TH; ++count){
			pthread_create(&(pth[count]), NULL, &write_th, NULL);//thread for write
		}
	}


	for (count = 0; count < QUAN_TH; ++count)
		pthread_join(pth[count], NULL);

	status = pthread_mutex_destroy(&mutex);
	if (status != 0) {
		perror("mutex_destroy");
		return 2;
	}

	return 0;
}
