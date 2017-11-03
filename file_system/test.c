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

#define MAX_BUF 1024
#define MAX_NAME 256
//keys
#define KEY_N "-n"
#define KEY_D "-d"
#define KEY_L "-l"
#define KEY_V "-v"
//colors
#define COLOR_END   "\x1b[0m"
#define COLOR_BLUE  "\x1b[34m"
#define COLOR_GREEN "\x1b[32m"

#define DEF_NAME "file2.gm11"

uint8_t magic[3] = {0x71, 0x77, 0x49};

struct GM_header {
	char* name;
	uint8_t magic[3];
};

struct File {
	size_t quan_ln;
	char* text;
	char name[256];
};

void create_SF (void* oSFile, int* quan_files){
	fwrite(magic, sizeof(uint8_t), sizeof(magic), oSFile);
	fwrite("\nlist: 0\n", sizeof(char), sizeof("\nlist: 0\n"), oSFile);
//	fprintf(oSFile, "\nlist: 0\n");
	printf("create gm11\n");
	*quan_files = 0;
}

bool check_SF (int* quan_files, void* oSFile, struct GM_header* gm_her){
	int i;
	if (*quan_files) {
		//gm11 exist
		size_t s;
		s = fread(gm_her->magic, sizeof(uint8_t), sizeof(magic), oSFile);
		if (s == sizeof(magic)){
			for (i = 0; i < (int)s; ++i){
				if (gm_her->magic[i] != magic[i]){
					printf("IT'S A TRAP (it isn't magic of .gm11)\n");
					*quan_files = 0;
					break;
				}
			}
		}
		else {
			printf("IT'S A TRAP (it isn't size of magic of .gm11)\n");
			*quan_files = 0;
		}
		char answer = 'n';
		if (!(*quan_files)){
			//UI (rewrite file)
			printf("You want rewrite %s file or not?(y/n): ", gm_her->name);
			scanf("%c", &answer);	
			if (answer != 'n'){
				printf("You answered %c (yes)\n", answer);
				fclose(oSFile);
				oSFile = fopen(DEF_NAME, "wb+");
				create_SF(oSFile, quan_files);
			} else
				return false;
		} else {
			//quantity of files in gm11
			char buf[sizeof("list:")];
			fscanf(oSFile, "%s%d", buf, quan_files);
			printf("%s %d\n", buf, *quan_files);
		}
	} else {
		//file don't exist
		create_SF(oSFile, quan_files);
	}
	return true;
}


int main(int argc, char** argv)
{
	//name of save_file
	char *gm11 = (char*) malloc(sizeof(DEF_NAME)+1);
	if (gm11 == NULL) exit (1);
	strcpy (gm11, DEF_NAME);

	struct GM_header gm_her;
	gm_her.name = gm11;
	char buf[MAX_BUF] = {0};
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

	//open or create save_file
	FILE *oSFile = fopen(gm11, "ab+");
	fseek(oSFile, 0, SEEK_SET);

	//we didn't have save_file
	if(!check_SF(&number_of_files, oSFile, &gm_her)){
		printf("Godspeed!\n");
		return 2;
	}

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
	}
	//close save_file
	fclose(oSFile);
	free(gm11);
	return 0;
}
