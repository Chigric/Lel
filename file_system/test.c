#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>

#include <dirent.h>
#include <sys/types.h>
#include <errno.h>


#define MAX_BUF 1024
#define KEY_N "-n"
#define KEY_D "-d"
#define KEY_L "-l"
//colors
#define COLOR_END   "\x1b[0m"
#define COLOR_BLUE  "\x1b[34m"
#define COLOR_GREEN "\x1b[32m"

#define DEF_NAME "file2.gm11"

uint8_t magic[3] = {0x71, 0x77, 0x49};

struct GM_header {
	uint8_t magic[3];
};

struct File {
	char* name;
	char* text;
	off_t size;
};

int main(int argc, char** argv)
{
	struct GM_header gm_her;
//	char buf[MAX_BUF] = {0};
        struct dirent *dp;
        DIR *dirp;	
	//name of save_file
	char *gm11= DEF_NAME;
	//count
	int ptr;

        if ((dirp = opendir(".")) == NULL) {
                perror("DON'T OPEN .\n");
                return 1;
        }
	int number_of_files = NULL;
        while ((dp = readdir(dirp)) != NULL) {
               	if (strcmp(dp->d_name, gm11) == 0){
			number_of_files = 1;
			printf("yes__");
		}
		else
			printf("no__");
		printf("%s\n", (char*)dp->d_name);
	}
	closedir(dirp);
	//open  save_file
	FILE *oSFile = fopen(gm11, "a+");
	if(oSFile == NULL){
                perror("FAIL WITH SAVE_FAIL!\n");
                return 1;
        }
	fseek(oSFile, 0, SEEK_SET);
	//we didn't have save_file
	if (!number_of_files) {
		fwrite(magic, sizeof(uint8_t), sizeof(magic), oSFile);
		fprintf(oSFile, "\n\tlist: 0\n\n");
		printf("no gm11\n");
	} else {
		size_t s;
		s = fread(gm_her.magic, sizeof(uint8_t), sizeof(magic), oSFile);
		if (s == sizeof(magic))
			for (ptr = 0; ptr < (int)s; ++ptr){
				if (gm_her.magic[ptr] != magic[ptr]){
					perror("IT'S A TRAP (it isn't magic of .gm11)\n");
					return 2;
				}
			}
		else {
			perror("IT'S A TRAP (it isn't size of magic of .gm11)\n");
			return 2;
		}
		printf("yes gm11 \n");
	}

	//close save_file
	fclose(oSFile);
	return 0;
}
