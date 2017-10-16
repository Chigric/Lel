#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_PATH 1024

#define COLOR_END   "\x1b[0m"
#define COLOR_BLUE  "\x1b[34m"
#define COLOR_GREEN "\x1b[32m"

int main(int argc, char **argv)
{
	struct stat stbuf;
        char buf[MAX_PATH] = {0};

        struct dirent *dp;
        DIR *dirp;

	uid_t uid = getuid();
	uid_t euid= geteuid();
	struct passwd *pas = getpwuid(uid);
	struct group *group;
	char last_mod[MAX_PATH];

        if ((dirp = opendir(".")) == NULL) {
        	fprintf(stderr, "don't open .\n");
                return 1;
        }
        while ((dp = readdir(dirp)) != NULL) {
        	if (strcmp(dp->d_name, ".") == 0
                    || strcmp(dp->d_name, "..") == 0)
                	continue;
		if (stat(dp->d_name, &stbuf) == -1) {
          		fprintf(stderr, "don't connect (stat()) with .\n");
		        return 2;
	        }
		char keys[sizeof('r')*10];
		//is folder?
		keys[0] = (S_ISDIR(stbuf.st_mode)) ?  'd' : '-';
		keys[1] = (stbuf.st_mode & S_IRUSR) ? 'r' : '-';
                keys[2] = (stbuf.st_mode & S_IWUSR) ? 'w' : '-';
                keys[3] = (stbuf.st_mode & S_IXUSR) ? 'x' : '-';
                keys[4] = (stbuf.st_mode & S_IRGRP) ? 'r' : '-';
                keys[5] = (stbuf.st_mode & S_IWGRP) ? 'w' : '-';
		keys[6] = (stbuf.st_mode & S_IXGRP) ? 'x' : '-';
                keys[7] = (stbuf.st_mode & S_IROTH) ? 'r' : '-';
                keys[8] = (stbuf.st_mode & S_IWOTH) ? 'w' : '-';
		keys[9] = (stbuf.st_mode & S_IXOTH) ? 'x' : '-';
		int i = 0;
		for (; i < 10; ++i) {
			buf[i] = keys[i];
		}
		strftime(last_mod, sizeof(last_mod)/sizeof(last_mod[0]), 
			 "%b %d %H:%M ", localtime(&(stbuf.st_mtime)));
		char name_f[MAX_PATH];
		sprintf(name_f, "%s %s %s", 
			(keys[0] == 'd') ? COLOR_BLUE : ((keys[9]=='x') ? COLOR_GREEN : COLOR_END), 
				dp->d_name, COLOR_END);
		group = getgrgid(stbuf.st_gid);
		printf("%s %3ld %s\t %7s  %7lld %s %s\n", buf, (long)stbuf.st_nlink,
			(uid == 0 || uid != euid) ? "root " : pas->pw_name,
			(uid == 0 || uid != euid) ? "root " : group->gr_name,
			(long long)stbuf.st_size, last_mod, name_f);
		
	}
	closedir(dirp);
	return 0;
} 

