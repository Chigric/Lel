#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	char buf[MAX_BUF];
	//name of save_file
	char *gm11= "file1.gm11";
	//open  save_file
	FILE *oSFile = fopen(gm11, "a+");
        if(oSFile == NULL){
                perror("FAIL WITH SAVE_FAIL!");
                return 1;
        }
	fseek(oSFile, 0, SEEK_SET);
//	if (
	//new_file
	struct File new_file;
	new_file.name = NULL;
	new_file.text = NULL;
	if (strcmp((char*)KEY_N, argv[1]) == 0)
	{
		new_file.name = argv[2];
		//open new_file
		FILE *oNFile = fopen(new_file.name, "r");
		if(oNFile == NULL){
	                perror("FAIL WITH NEW_FAIL!");
                	return 1;
        	}
		size_t s;
		fprintf(oSFile, "\n\nfile:\n\tname file: %s\n", new_file.name);
		while (!feof(oNFile))
		{
			s = fread(buf, sizeof(char), MAX_BUF, oNFile);
			fwrite(buf, sizeof(char), s, oSFile);
		}
		fprintf(oSFile, "\nEND FILE ---");
		//close new_file
		fclose(oNFile);
	} else if (strcmp((char*)KEY_D, argv[1]) == 0)
	{
		//delete argv[1]
	} else if (strcmp((char*)KEY_L, argv[1]) == 0)
	{
		//view list of files in save_file
	}
	printf("%d %s\n", argc, new_file.name);
	//close save_file
	fclose(oSFile);
	return 0;
}
