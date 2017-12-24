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
//memcpy(), strcat()
#include <stdbool.h>
//bool
#include <unistd.h>
//sleep()
#include <pthread.h>
//pthread, mutex and conditional variable

#define COLOR_RED   	"\x1b[31m"
#define COLOR_END   	"\x1b[0m"
#define SIZE 		16 * 1024
#define QUAN_RD_TH 	2//quantity reading threads
#define QUAN_WRT_TH 	1//quantity writing threads

//shared memory
void* shmlink;
int shmid;
bool flag = true, full = false;
//mutex
pthread_mutex_t mutex;
//The conditional variable
pthread_cond_t cond;
//threads
pthread_t rd[QUAN_RD_TH];
pthread_t wrt[QUAN_WRT_TH];


void signal_handler(int signal)
{
	if (signal == SIGINT) {	
		shmdt(shmlink);//attaches the shared memory
		shmctl(shmid, IPC_RMID, NULL);//mark the segment to be destroyed
		exit(EXIT_SUCCESS);
	}
}

void* read_th(void* c_)
{//read from shared memory and print to term.
	while(true) {
		sleep(1);
		pthread_mutex_lock(&mutex);
		while (full == true)
			pthread_cond_wait(&cond, &mutex);
		printf("\n%s\n", (char*)shmlink);
		full = true;
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void* write_th(void* c_)
{//wtire to shared memory
	while(true) {
		pthread_mutex_lock(&mutex);
		memcpy((char*)shmlink, COLOR_RED, strlen(COLOR_RED)+1);
		int i = strlen(COLOR_RED);
		for (; i < SIZE - strlen(COLOR_END)+1; ++i){
			if (i%100 == 0)
				((char*)shmlink)[i] = 'D';
			else
				((char*)shmlink)[i] = 'k';
		}
		strcat((char*)shmlink, COLOR_END);
		full = false;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		sleep(1);
	}
	return NULL;
}

int main()
{
	//magic. Now it is'n magic(.
	signal(SIGINT, signal_handler);

	int status;//check
	status = pthread_mutex_init(&mutex, NULL);
	if (status != 0) {
		perror("pthread_mutex_init");
		return 2;
	}

	int	id = 6;
	key_t key = ftok("../README.md", id);
	if (key == -1){
		perror("ftok");
		return 1;
	}
	printf("%x\n", (unsigned int)key);
	
	//+1 - '\0'
	shmid = shmget(key, SIZE+1, IPC_CREAT | 0776);
	if (shmid == -1) {
		perror("shmget");
		return 1;
	}
	flag = false;

	shmlink = shmat(shmid, NULL, 0);
	if (shmlink == (void *) -1) {
		perror("shmat");
		return 1;
	}
	
	int count;
	printf("read:\n");
	for(count = 0; count < QUAN_RD_TH; count++){
		pthread_create(&(rd[count]), NULL, &read_th, NULL);//thread for read
	}
	printf("write:\n");
	for(count = 0; count < QUAN_WRT_TH; ++count){
		pthread_create(&(wrt[count]), NULL, &write_th, NULL);//thread for write
	}

	for (count = 0; count < QUAN_RD_TH; ++count)
		pthread_join(rd[count], NULL);

	for (count = 0; count < QUAN_WRT_TH; ++count)
		pthread_join(wrt[count], NULL);

	status = pthread_mutex_destroy(&mutex);
	if (status != 0) {
		perror("mutex_destroy");
		return 2;
	}
	status = pthread_cond_destroy(&cond);
	if (status != 0) {
		perror("cond_destroy");
		return 2;
	}

	return 0;
}
