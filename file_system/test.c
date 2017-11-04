#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <stdbool.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_BUF 1024
#define MAX_NAME 256
//keys
#define KEY_N "-n"
#define KEY_D "-d"
#define KEY_L "-l"
#define KEY_V "-v"
#define KEY_R "-r"
//colors
#define COLOR_END   "\x1b[0m"
#define COLOR_BLUE  "\x1b[34m"
#define COLOR_GREEN "\x1b[32m"

#define DEF_NAME "file2.gm11"
#define DEF_QUAN_MAGIC 3

typedef enum {
	K_NONE,
	K_NEW,
	K_DELETE,
	K_LIST,
	K_VIEW,
	K_RENAME
} GM_KEYS;

uint8_t magic[DEF_QUAN_MAGIC] = {0x71, 0x77, 0x49};

struct GM_header {
	char* name;
	uint8_t magic[DEF_QUAN_MAGIC];
	off_t off;
};

struct File {
	size_t size;
	char* text;
	char name[MAX_NAME];
};

void create_SF (int* oSFile, int* quan_files){
	//create new save_file
	*oSFile = open(DEF_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD);
	if (*oSFile < 0) {
		perror("DON'T CREAT SAVE_FILE");
		exit(0);
	}
	int ptr = 0;
	//fprintf(oSFile, "\nlist: 0\n");
	write(*oSFile, magic, sizeof(magic));
//	write(*oSFile, "\nlist: 1\n", sizeof("\nlist: \n") + 1);
	write(*oSFile, "list: ", sizeof("list: ") - 1);
	write(*oSFile, &ptr, sizeof(int));
//	write(*oSFile, "\n", sizeof(char));
	printf("create gm11\n");
	*quan_files = 0;
}

bool check_SF (int* quan_files, int* oSFile, struct GM_header* gm_her){
	int ptr;
	if (*quan_files) {
		//gm11 exist
		ssize_t siz = NULL;
		if ((siz = read(*oSFile, gm_her->magic, sizeof(magic))) != sizeof(magic)){
				fprintf(stderr, "CAN'T READ MAGIC\n");
				perror("\n");
		}
		printf("check_SF:\nsiz  = %zu\tsizeof(magic) = %li\n", siz, sizeof(magic));
		if (siz == sizeof(magic)){
			for (ptr = 0; ptr < (int)siz; ++ptr){
				if (gm_her->magic[ptr] != magic[ptr]){
					fprintf(stderr, "IT'S A TRAP (it isn't magic of .gm11)\n");
					*quan_files = 0;
					break;
				}
			}
		}
		else {
			fprintf(stderr, "IT'S A TRAP (it isn't size of magic of .gm11)\n");
			*quan_files = 0;
		}

		char answer = 'n';
		if (!(*quan_files)){
			//UI (rewrite file)
			printf("You want rewrite %s file or not?(y/n): ", gm_her->name);
			scanf("%c", &answer);	
			if (answer != 'n'){
				printf("You answered %c (yes)\n", answer);
				close(*oSFile);
				create_SF(oSFile, quan_files);
			} else
				return false;
		} else 	//quantity of files in gm11
			return true;	
	} else {
		//file don't exist
		create_SF(oSFile, quan_files);
	}
	return true;
}


int main(int argc, char** argv)
{
	//some bufer
	char buf[MAX_BUF] = {0};

	//name of save_file
	char *gm11 = (char*) malloc(sizeof(DEF_NAME)+1);
	if (gm11 == NULL) exit (1);
	strcpy (gm11, DEF_NAME);

	//some inf about save_file
	GM_KEYS key = K_NONE;
	struct GM_header gm_her;
	gm_her.name = gm11;

	//directory
        struct dirent *dp;
        DIR *dirp;
        if ((dirp = opendir(".")) == NULL) {
                perror("DON'T OPEN .\t");
                return 1;
        }
	int number_of_files = NULL;
        while ((dp = readdir(dirp)) != NULL){
               	if (strcmp(dp->d_name, gm11) == 0)
			number_of_files = 1;
	}
	closedir(dirp);

	//open save_file
	int oSFile;
	if ((oSFile = open(gm11, O_RDWR)) == -1) {
		perror("DON'T OPEN SAVE FILE");
	} else	lseek(oSFile, 0, SEEK_SET);

	//we didn't have save_file
	if(!check_SF(&number_of_files, &oSFile, &gm_her)){
		fprintf(stderr, "Godspeed!\n");
		return 2;
	}

	//goto number of list
	if ((gm_her.off = lseek(oSFile, sizeof(magic) + sizeof("list: ") - 1, SEEK_SET)) == -1)
		perror("LSEEK AFTER CHECK");

	if (argc > 1) {
		if (strcmp((char*)KEY_N, argv[1]) == 0)
		{	//add new_file
			key = K_NEW;
			
			if (read(oSFile, &number_of_files, sizeof(int)) <= 0){
				fprintf(stderr, "CAN'T READ SIZE OF LIST");
				perror("\n");
			}
			++number_of_files;

			lseek(oSFile, -sizeof(int), SEEK_CUR);
			if (write(oSFile, &number_of_files, sizeof(int)) <= 0){
				fprintf(stderr, "CAN'T WRITE SIZE OF LIST");
				perror("\n");
			}
//			lseek(oSFile, sizeof("\n"), SEEK_CUR);
			printf("<<_IN:\targc > 1\nbuf\t, number_of_files:\n%s\t, %d\n", buf, number_of_files);		

		} else if (strcmp((char*)KEY_D, argv[1]) == 0)
		{	//delete argv[2]
			key = K_DELETE;
		} else if (strcmp((char*)KEY_L, argv[1]) == 0)
		{	//view list of files in save_file
			key = K_LIST;

			if (read(oSFile, &number_of_files, sizeof(int)) <= 0){
				fprintf(stderr, "CAN'T READ SIZE OF LIST");
				perror("\n");
			}	
//			lseek(oSFile, sizeof("\n"), SEEK_CUR);
			printf("<<_IN:\targc > 1\nbuf\t, number_of_files:\n%s\t, %d\n", buf, number_of_files);

			
		} else if (strcmp((char*)KEY_V, argv[1]) == 0)
		{	//view argv[2]
			key = K_VIEW;
		} else if (strcmp((char*)KEY_R, argv[1]) == 0)
		{	//rename save_file
			key = K_RENAME;
		}
	}
/*
	//NEW_file
	struct File new_file;
	new_file.text = NULL;
	new_file.quan_ln = 0;
	char** list_files = NULL;
	if (argc == 3) {
		if (strcmp((char*)KEY_N, argv[1]) == 0)
		{
			//add NEW_file to Save_file
			struct stat stbuf;
			strcpy(new_file.name, argv[2]);
			if (stat(argv[2], &stbuf) == -1) {
        	  		perror("don't connect (stat()) with .\n");
			        return 2;
		        }
			new_file.quan_ln = stbuf.st_size;
			fprintf(oSFile, "%s %d\n", new_file.name, new_file.quan_ln);
			//open new_file
			FILE *oNFile = fopen(new_file.name, "r");
			if(oNFile == NULL){
	        	        perror("FAIL WITH NEW_FAIL!\n");
                		return 1;
	        	}
			size_t s;
			fprintf(oSFile, "\n\tname file: %s\tsize: %d\n", new_file.name, new_file.quan_ln);
			while (!feof(oNFile))
			{
				s = fread(buf, sizeof(char), MAX_BUF, oNFile);
				fwrite(buf, sizeof(char), s, oSFile);
			}
			fseek(oSFile, sizeof(uint8_t)*3 + sizeof("\nlist: "), SEEK_SET);
			//close new_file
			fclose(oNFile);
		} else if (strcmp((char*)KEY_D, argv[1]) == 0)
		{
			//delete argv[1]
		} else if (strcmp((char*)KEY_L, argv[1]) == 0)
		{
			//view list of files in save_file
		} else if (strcmp((char*)KEY_V, argv[1]) == 0)
		{
			//view argv[2]
		}
	}

	if (list_files != NULL)
	{//free list_files
		int i;
		for (i = 0; i < number_of_files; i++)
			free(list_files[i]);
		free(list_files);
	}*/
	//close save_file
	close(oSFile);
	free(gm11); 
	return 0;
}
