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
#define KEY_F "-f"
#define KEY_D "-d"
#define KEY_L "-l"
#define KEY_V "-v"
#define KEY_R "-r"
//colors
#define COLOR_END   	"\x1b[0m"
#define COLOR_RED   	"\x1b[31m"
#define COLOR_GREEN	"\x1b[32m"
#define COLOR_YELLOW 	"\x1b[33m"
#define COLOR_BLUE	"\x1b[34m"
#define COLOR_MAGENTA	"\x1b[35m"
#define COLOR_CYAN  	"\x1b[36m"

#define MAX_COLOR 8
#define DEF_NAME "file2.gm11"
#define DEF_QUAN_MAGIC 3

typedef enum {
	K_NONE,
	K_NEW,
	K_FREE,
	K_DELETE,
	K_LIST,
	K_VIEW,
	K_RENAME
} GM_KEYS;

uint8_t magic[DEF_QUAN_MAGIC] = {0x71, 0x77, 0x49};

struct GM_header {
	uint8_t magic[DEF_QUAN_MAGIC];
	char* name;
	off_t off;
};

struct File {
	bool deleted;
	uint8_t len_name;	
	off_t size;
	char* name;
};

size_t size_st_f(struct File* _f){
	return (_f->len_name * sizeof(char) +
		sizeof(off_t) + sizeof(uint8_t) +
		sizeof(bool));
}

void print_st_f(struct File _f) {
	printf("%sdeleted = %d size of name = %d size of file = %7zu%7s%s%s\n", 
		COLOR_CYAN, _f.deleted, _f.len_name, _f.size,
		COLOR_GREEN, _f.name, COLOR_END);
}

void write_st_f(int to_file, struct File from_f) {
 	if (write(to_file, &(from_f.deleted), sizeof(bool)) <= 0){
		fprintf(stderr, "CAN'T WRITE NEW STRUCTURE");
		perror("\n");
	}
 	if (write(to_file, &(from_f.len_name), sizeof(uint8_t)) <= 0){
		fprintf(stderr, "CAN'T WRITE NEW STRUCTURE");
		perror("\n");
	}
 	if (write(to_file, &(from_f.size), sizeof(off_t)) <= 0){
		fprintf(stderr, "CAN'T WRITE NEW STRUCTURE");
		perror("\n");
	}
 	if (write(to_file, (from_f.name), sizeof(char) * from_f.len_name) <= 0){
		fprintf(stderr, "CAN'T WRITE NEW STRUCTURE");
		perror("\n");
	}
}

bool read_st_f(int main_f, struct File* _cur) {
	if(read(main_f, &(_cur->deleted), sizeof(bool)) == 0){
		//END of main_f
		fprintf(stderr, "%d ", main_f);
		perror("#1\n");
		return false;
	}
	if(read(main_f, &(_cur->len_name), sizeof(uint8_t)) == 0){
		perror("#2\n");
		return false;
	}
	if(read(main_f, &(_cur->size), sizeof(off_t)) == 0){
		perror("#3\n");	
		return false;
	}
	if (_cur->name != NULL)
		//!!!MALLOC!!!
		free(_cur->name);
	_cur->name = (char*)malloc(1 + _cur->len_name);	
	if (_cur->name == NULL){ 
		perror("MALLOC IN READ MAIN FAIL");
		exit(2);
	}
	if(read(main_f, (_cur->name), sizeof(char) * _cur->len_name) == 0) {
		perror("#4\n");
		return false;
	}
	return true;
}

void rewrite_mu_f(int oSFile, off_t size_ff, off_t size_tf){
	{
		off_t ttt;
		lseek(oSFile, -1, SEEK_CUR);
		ttt = lseek(oSFile, 1, SEEK_CUR);
		printf("\n\trewrite 1: lseek = %zu\n", ttt);
	
	ttt = lseek(oSFile, -size_ff, SEEK_CUR);

		printf("\n\trewrite 2: lseek = %zu\n", ttt);
	}
	//create new buf for more fast copy
	size_t size_buf_plus = size_ff;
	//!!!MALLOC!!!
	char* buf_plus = (char*)malloc(size_buf_plus);
	if (buf_plus == NULL) {
		/////////////////////////////////////////
		perror("MALLOC(BUF_SIZE)");
		/////////////////////////////////////////
		size_buf_plus = MAX_BUF;
		buf_plus = (char*)malloc(size_buf_plus);
		if (buf_plus == NULL) {
			perror("COMPUTER HAVEN'T MEMORY FOR THIS OPERATION");
			exit(2);
		}
	}
	////////////////////////////////////////////////////////////////////////////////////
	fprintf(stderr, "size_buf_plus = %zu\n", size_buf_plus);
	////////////////////////////////////////////////////////////////////////////////////
	
	//rewrite block of NOT deleted files
	size_ff -= read(oSFile, buf_plus, size_buf_plus);
	do {
		off_t sss;
		lseek(oSFile, -1, SEEK_CUR);
		sss = lseek(oSFile, 1, SEEK_CUR);
		printf("\t1: lseek = %zu\n", sss);
		sss = lseek(oSFile, -(size_tf + size_buf_plus), SEEK_CUR);
		printf("\t2: lseek = %zu\n", sss);
		ssize_t fff;
		fff = write(oSFile, buf_plus, size_buf_plus);
		printf("\t   write = %zu\n", fff);	
		sss = lseek(oSFile, size_tf, SEEK_CUR);
		printf("\t3: lseek = %zu\n", sss);
		if (size_buf_plus > size_ff) size_buf_plus = size_ff;
	} while (size_ff -= (read(oSFile, buf_plus, size_buf_plus)) > 0);
	off_t ggg;
		ggg = lseek(oSFile, -(size_tf), SEEK_CUR);
		printf("\n\trewrite 3: lseek = %zu\n", ggg);
	//FREE!!!
	free(buf_plus);
}

bool search_st_f(int save_f, char* new_f, struct File* _cur, struct GM_header* gm){
	lseek(save_f, (gm->off + sizeof(int)), SEEK_SET);
	while(read_st_f(save_f, _cur)){
		if((_cur->deleted == false) && (strcmp(_cur->name, new_f) == 0)){
			return true;
		} else lseek(save_f, _cur->size, SEEK_CUR);
	}
	return false;
}

void create_SF (int* oSFile, int* quan_files){
	//create new save_file
	*oSFile = open(DEF_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IWRITE | S_IREAD);
	if (*oSFile < 0) {
		perror("DON'T CREAT SAVE_FILE");
		exit(0);
	}
	int ptr = 0;
	write(*oSFile, magic, sizeof(magic));
	write(*oSFile, &ptr, sizeof(int));
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

	int number_of_files = NULL;
	//open save_file
	int oSFile;
	if ((oSFile = open(gm11, O_RDWR)) == -1) {
		perror("DON'T OPEN SAVE FILE");
	} else number_of_files = 1;
	
	//we didn't have save_file
	if(!check_SF(&number_of_files, &oSFile, &gm_her)){
		fprintf(stderr, "Godspeed!\n");
		return 2;
	}

	//goto quantity of files of list
	if ((gm_her.off = lseek(oSFile, sizeof(magic), SEEK_SET)) == -1)
		perror("LSEEK AFTER CHECK");

	//read number of files in SAVE_file
	if (read(oSFile, &number_of_files, sizeof(int)) <= 0){
		fprintf(stderr, "CAN'T READ SIZE OF LIST");
		perror("\n");
	}

	if (argc > 1) {
		if (strcmp((char*)KEY_N, argv[1]) == 0)
		{	//add new_file
			key = K_NEW;

			//write new_file
			int ptr;
			for (ptr = 2; ptr < argc; ++ptr) {
				lseek(oSFile, gm_her.off, SEEK_SET);
				//augment number of files in SAVE_file
				++number_of_files;
				if (write(oSFile, &number_of_files, sizeof(int)) <= 0){
					fprintf(stderr, "CAN'T WRITE SIZE OF LIST");
					perror("\n");
				}

				//creat structure of NEW_file
				struct File new_file;
				new_file.size = 0;
				new_file.deleted = false;
				new_file.len_name = 0x00;
				new_file.name = NULL;

				//fill the structure
				struct stat stbuf;
				new_file.len_name = (uint8_t)strlen(argv[ptr]);
				//!!!MALLOC!!!
				new_file.name = (char*)malloc(strlen(argv[ptr]) + 1);
				if(new_file.name == NULL) {
					perror("HOUSTON, WE HAVE A PROBLEM (-n, malloc)"); return 1;
				}
				strcpy(new_file.name, argv[ptr]);
				if (stat(argv[ptr], &stbuf) == -1) {
        		  		perror("DON'T CONNECT (STAT()) WITH . (-n)\n");	return 2;
		        	}
				new_file.size = stbuf.st_size;

				//write structure of NEW_file to SAVE_file
				//search NEW_file in SAVE_file
				//!!!MALLOC!!
				struct File* sel_f = (struct File*)malloc(sizeof(struct File));
				sel_f->size = 0;
				sel_f->deleted = false;
				sel_f->len_name = 0x00;
				sel_f->name = NULL;
				if (search_st_f(oSFile, new_file.name, sel_f, &gm_her)) {
					//some UI
					char answer;
					printf("File with this name exist. You want rewrite his (y/n)?\n");
					scanf("%c%*c", &answer);
					if(answer == 'n'){
						//goto  quantity of files of list
						if ((gm_her.off = lseek(oSFile, sizeof(magic), SEEK_SET)) == -1)
							perror("LSEEK, AFTER ANSWER (-n)");
						--number_of_files;
						if (write(oSFile, &number_of_files, sizeof(int)) <= 0){
							fprintf(stderr, "CAN'T WRITE SIZE OF LIST, AFTER ANSWER");
							perror("\n");
						}
						//FREE!!!
						free(sel_f->name);
						//FREE!!!
						free(sel_f);
						continue;
					} else {//delete searched file
						lseek(oSFile, -(sel_f->len_name * sizeof(char) +
							sizeof(off_t) + sizeof(uint8_t) +
							sizeof(bool)),
							SEEK_CUR);
						sel_f->deleted = true;
						write(oSFile, &(sel_f->deleted), sizeof(bool));
					}
				}
				//FREE!!!
				free(sel_f->name);
				//FREE!!!
				free(sel_f);

				//open new_file
				int oNFile;
				if ((oNFile = open(new_file.name, O_RDONLY)) == -1){
					perror("DON'T OPEN NEW FILE");
					return 1;
				}
				//write new_file to END of save_file
				lseek(oSFile, 0, SEEK_END);
				write_st_f(oSFile, new_file);
				//rewrite text from NEW_file to SAVE_file
				ssize_t siz;
				while ((siz = read(oNFile, buf, MAX_BUF)) > 0)
					write(oSFile, buf, siz);
				if (siz == -1){
					perror("CAN'T READ NEW FILE");
					return 1;
				}
				//FREE!!!
				free(new_file.name);
				//close new_file
				close(oNFile);
			}

		} else if (strcmp((char*)KEY_F, argv[1]) == 0)
		{	//free argv[2]
			key = K_FREE;

			//read list of SAVE_file
			int cur_;
			struct File cur_f;
			cur_f.deleted = false;
			cur_f.len_name = 0x00;
			cur_f.size = 0;
			cur_f.name = NULL;
			//some variables (=_=)
			int quan_del_files = 0;
			off_t len_del_files = 0;
			int quan_tf = 0;
			int quan_ff = 0;
			off_t size_ff = 0;
			off_t size_tf = 0;
			for (cur_ = 0; cur_ < number_of_files; ++cur_){
				//read cur file
				if (!read_st_f(oSFile, &cur_f)) {
					perror("CANN'T READ SAVE FILE (-f)"); 
					print_st_f(cur_f);
					break;
				}
				print_st_f(cur_f);
				
				if (cur_f.deleted) {
					//true
					if (quan_ff > 0) {
						lseek(oSFile, -(size_st_f(&cur_f)), SEEK_CUR);
						{
							off_t ttt;
							lseek(oSFile, -1, SEEK_CUR);
							ttt = lseek(oSFile, 1, SEEK_CUR);
							printf("\n\t\t\tlseek = %zu\n", ttt);
						}
						printf("true:\nsize_ff = %zu\t size_tf = %zu\n", size_ff, size_tf);
						rewrite_mu_f(oSFile, size_ff, size_tf);
						//quantity of deleted files in this block
						quan_del_files += quan_tf;
						//as we add-on size_tf to next structure
						//len_del_files += size_tf;

						//move next deleted(true) structure to last NOT deleted structure
						//(i.e. to back)
						//Back To The Future, Morty
						cur_f.size += size_tf;
						write_st_f(oSFile, cur_f);
						//zeroing out
						size_ff = 0; quan_ff = 0;
						size_tf = 0; quan_tf = 0;
					}
					
					++quan_tf;
					size_tf += size_st_f(&cur_f);
					////////////////////////////////////////////////////
					printf("size_tf = %zu\t", size_tf);
					////////////////////////////////////////////////////

					lseek(oSFile, cur_f.size, SEEK_CUR);
					{
						off_t sss;
						lseek(oSFile, -1, SEEK_CUR);
						sss = lseek(oSFile, 1, SEEK_CUR);
						printf("\n\tcur_f.size = %zu\n", cur_f.size);
						printf("\n\ttrue lseek = %zu\n", sss);
					}
					size_tf += cur_f.size;

					////////////////////////////////////////////////////
					printf("size_tf = %zu\n", size_tf);
					////////////////////////////////////////////////////	
				} else {
					 if(quan_tf > 0) {
						//false
						++quan_ff;
						size_ff += size_st_f(&cur_f);
						////////////////////////////////////////////////////
						printf("size_ff = %zu\t", size_ff);
						////////////////////////////////////////////////////
						
						lseek(oSFile, cur_f.size, SEEK_CUR);
						{
							off_t sss;
							lseek(oSFile, -1, SEEK_CUR);
							sss = lseek(oSFile, 1, SEEK_CUR);
							printf("\n\tfalse lseek = %zu", sss);
						}
						size_ff += cur_f.size;

						////////////////////////////////////////////////////
						printf("size_ff = %zu\n", size_ff);
						////////////////////////////////////////////////////
						//It's last deleted(false) file
						if ((cur_ + 1 == number_of_files)){
							rewrite_mu_f(oSFile, size_ff, size_tf);
							quan_del_files += quan_tf;
							len_del_files += size_tf;
							//zeroing out
							quan_tf = 0; size_tf = 0;
						}
					} else {
						lseek(oSFile, cur_f.size, SEEK_CUR);
						{
							off_t sss;
							lseek(oSFile, -1, SEEK_CUR);
							sss = lseek(oSFile, 1, SEEK_CUR);
							printf("\n\tfalse false lseek = %zu", sss);
						}
					}
				}
				{
					off_t ttt;
					lseek(oSFile, -1, SEEK_CUR);
					ttt = lseek(oSFile, 1, SEEK_CUR);
					printf("\n\t\t\tlseek = %zu", ttt);
					printf("quan_del_files = %d\n", quan_del_files);
				}
				//It's last file
				if (cur_ + 1 == number_of_files){
					if (quan_tf > 0) {
						//quantity of deleted files in this block
						quan_del_files += quan_tf;
						printf("quan_del_files = %d\n", quan_del_files);
						len_del_files += size_tf;
					}
					struct stat stbuf;
					if (stat(gm_her.name, &stbuf) == -1) {
			        		perror("DON'T CONNECT (STAT()) WITH . (-f)\n");
					        return 2;
		        		}
					if(ftruncate(oSFile, (stbuf.st_size - len_del_files)) == -1){
						///////////////////////////////////////////////////////////////////////////////////////////////
						fprintf(stderr, "stbuf.st_size = %zu\tlen_del_files = %zu\nstbuf.st_size - len_del_files) = %zu\n", 
							stbuf.st_size, len_del_files, (stbuf.st_size - len_del_files));
						///////////////////////////////////////////////////////////////////////////////////////////////
						perror("TRUNCATE IS INVALID");
						return 2;
					} else {
						///////////////////////////////////////////////////////////////////////////////////////////////
						fprintf(stderr, "stbuf.st_size = %zu\tlen_del_files = %zu\nstbuf.st_size - len_del_files) = %zu\n", 
							stbuf.st_size, len_del_files, (stbuf.st_size - len_del_files));
						///////////////////////////////////////////////////////////////////////////////////////////////
					}
				}
			}
			//FREE!!!
			free(cur_f.name);

			//rewrite number_of_files
			lseek(oSFile, gm_her.off, SEEK_SET);
			number_of_files -= quan_del_files;
			if (write(oSFile, &number_of_files, sizeof(int)) <= 0){
				fprintf(stderr, "CAN'T WRITE SIZE OF LIST");
				perror("\n");
			}
		}else if (strcmp((char*)KEY_D, argv[1]) == 0)
		{	//delete argv[2]
			key = K_DELETE;

			int ptr;
			for (ptr = 2; ptr < argc; ++ptr){
				bool same_file = false;
				if (ptr > 2) {
					int coun;
					for (coun = 2; coun < ptr; ++coun) {
						if (strcmp(argv[coun], argv[ptr]) == 0) {
							same_file = true;
							fprintf(stderr, "You print some same files \"%s\", WHY?\n", argv[coun]);
							break;
						}
					}
				}
				if (!same_file) {
					//!!!MALLOC!!
					struct File* sel_f = (struct File*)malloc(sizeof(struct File));
					sel_f->size = 0;
					sel_f->deleted = false;
					sel_f->len_name = 0x00;
					sel_f->name = NULL;
					if (search_st_f(oSFile, argv[ptr], sel_f, &gm_her)) 
					{ 	//delete searched file
						lseek(oSFile, -(sel_f->len_name * sizeof(char) +
							sizeof(off_t) + sizeof(uint8_t) +
							sizeof(bool)),
							SEEK_CUR);
						sel_f->deleted = true;
						write(oSFile, &(sel_f->deleted), sizeof(bool));
					} else 
						fprintf(stderr, "save file havn't \"%s\" file\n", sel_f->name);
					//FREE!!!
					free(sel_f->name);
					//FREE!!!
					free(sel_f);
				}
			}
		} else if (strcmp((char*)KEY_L, argv[1]) == 0)
		{	//view list of files in save_file
			key = K_LIST;

			printf("%stotal:%s %d%s\n", COLOR_RED, COLOR_MAGENTA, 
				number_of_files, COLOR_END);
			//read list of SAVE_file
			int cur_;
			struct File cur_f;
			for (cur_ = 0; cur_ < number_of_files; ++cur_){
				//read cur file
				if (!read_st_f(oSFile, &cur_f)) {perror("\n");break;}
				print_st_f(cur_f);
				lseek(oSFile, cur_f.size, SEEK_CUR);
			}
		} else if (strcmp((char*)KEY_V, argv[1]) == 0)
		{	//view argv[2]
			key = K_VIEW;
			
			int ptr;
			for (ptr = 2; ptr < argc; ++ptr) {
				bool same_file = false;
				if (ptr > 2) {
					int coun;
					for (coun = 2; coun < ptr; ++coun) {
						if (strcmp(argv[coun], argv[ptr]) == 0) {
							same_file = true;
							fprintf(stderr, "You print some same files \"%s\", WHY?\n", argv[coun]);
							break;
						}
					}
				}
				if (!same_file) {
					//!!!MALLOC!!
					struct File* sel_f = (struct File*)malloc(sizeof(struct File));
					sel_f->size = 0;
					sel_f->deleted = false;
					sel_f->len_name = 0x00;
					sel_f->name = NULL;
					if (search_st_f(oSFile, argv[ptr], sel_f, &gm_her)){
						print_st_f(*sel_f);
						ssize_t siz;
						off_t size_f = sel_f->size;
						while (size_f > 0){
							if ((siz = read(oSFile, buf, MAX_BUF)) < 0){
								perror("READ SAVE FILE");
								return 1;
							}
							printf("%s", buf);
							size_f -= siz;
						}
						printf("\n");
						if (siz == -1){
							perror("CAN'T READ NEW FILE");
							return 1;
						}
					} else 
						printf("save file havn't \"%s\" file\n", argv[ptr]);
					//FREE!!!
					free(sel_f->name);
					//FREE!!!
					free(sel_f);
				}
			}
		} else if (strcmp((char*)KEY_R, argv[1]) == 0)
		{	//rename save_file
			key = K_RENAME;
		}
	}
	//close save_file
	close(oSFile);
	free(gm11); 
	return 0;
}
