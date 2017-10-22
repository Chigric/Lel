#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <dirent.h>
#include <sys/types.h>
#include <errno.h>


#define MAX_BUF 1024
#define KEY_N "-n"
#define KEY_D "-d"
#define KEY_L "-l"

struct File {
	char* name;
	char* text;
};
//colors
int main(int argc, char** argv)
{
	char buf[MAX_BUF] = {0};
        struct dirent *dp;
        DIR *dirp;

        if ((dirp = opendir(".")) == NULL) {
                fprintf(stderr, "don't open .\n");
                return 1;
        }
	int numbers_of_files = NULL;
        while ((dp = readdir(dirp)) != NULL) {
               	if (strcmp(dp->d_name, "file1.gm11") == 0) 
			numbers_of_files = 1;
		else
			printf("no__");
		printf("%s\n", (char*)dp->d_name);

	}
	closedir(dirp);
	//name of save_file
	char *gm11= "file1.gm11";
	//open  save_file
	FILE *oSFile = fopen(gm11, "a+");
        if(oSFile == NULL){
                perror("FAIL WITH SAVE_FAIL!\n");
                return 1;
        }
	fseek(oSFile, 0, SEEK_SET);
	if (!numbers_of_files) {
		fprintf(oSFile, "\n\tlist: 0\n\n---END_LIST\n");
		printf("no gm11\n");
	} else printf("yes gm11 %d\n", numbers_of_files);
	//NEW_file
	struct File new_file;
	new_file.name = NULL;
	new_file.text = NULL;
	char** list_files;
	if (strcmp((char*)KEY_N, argv[1]) == 0)
	{
		if (numbers_of_files) {
			fscanf(oSFile, "%s%d", buf, &numbers_of_files);
			int ptr = numbers_of_files;
			list_files = (char**)malloc(ptr * sizeof(char*));
			for (; ptr < 0; --ptr) {
				if (fgets(buf, 100, oSFile) == NULL )
                        		fprintf(stderr, "Bad read (list)\n");
				list_files[ptr-1] = buf;
			}
		}		
		new_file.name = argv[2];
		//open new_file
		FILE *oNFile = fopen(new_file.name, "r");
		if(oNFile == NULL){
	                perror("FAIL WITH NEW_FAIL!\n");
                	return 1;
        	}
		size_t s;
		fprintf(oSFile, "\n\nfile:\n\tname file: %s\n", new_file.name);
		while (!feof(oNFile))
		{
			s = fread(buf, sizeof(char), MAX_BUF, oNFile);
			fwrite(buf, sizeof(char), s, oSFile);
		}
		fprintf(oSFile, "\nEND FILE ---\n");
		//close new_file
		fclose(oNFile);
	} else if (strcmp((char*)KEY_D, argv[1]) == 0)
	{
		//delete argv[1]
	} else if (strcmp((char*)KEY_L, argv[1]) == 0)
	{
		//view list of files in save_file
	}
	int i;
	for (i = 0; i < numbers_of_files; i++)
		free(list_files[i]);
	free(list_files);
	//close save_file
	fclose(oSFile);
	return 0;
}
