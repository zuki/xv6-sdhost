#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>


void format_file_mode(mode_t mode, char *buffer)
{
    mode_t curbit = 0400;

    strcpy(buffer, "-rwxrwxrwx");

    if (S_ISDIR(mode))
        buffer[0] = 'd';
    if (S_ISCHR(mode))
        buffer[0] = 'c';

    for (char i = 1; i < 10; i++) {
        if (!(mode & curbit))
            buffer[i] = '-';
        curbit >>= 1;
    }
}

int main(int argc, char **argv, char **envp)
{
    DIR *dir;
    int error;
    struct dirent *dent;
    struct stat statbuf;
    char filemode[10];
    char filename[100];
    char timestamp[100];

    char *path = argc > 1 ? argv[1] : ".";

    if ((dir = opendir(path)) < 0) {
        perror(argv[0]);
        return(EXIT_FAILURE);
    }

    int start = strlen(path) - 1;
    strcpy(filename, path);
    if (filename[start] != '/')
        filename[++start] = '/';
    start++;

    while ((dent = readdir(dir))) {
        //if (dent->d_name[0] != '.') {
            strcpy(&filename[start], dent->d_name);
            error = stat(filename, &statbuf);
            if (error < 0) {
                printf("Error at stat %s (%d)\n", filename, error);
                return error;
            }

            format_file_mode(statbuf.st_mode, filemode);
            strftime(timestamp, 100, "%Y-%m-%d %H:%M:%S", gmtime(&statbuf.st_mtime));
            if (S_ISBLK(statbuf.st_mode) || S_ISCHR(statbuf.st_mode))
                printf("%s %2d, %2d %s %s\n", filemode, (statbuf.st_rdev >> 8) & 0x00ff, statbuf.st_rdev & 0x00ff, timestamp, dent->d_name);
            else
                printf("%s %6d %s %s\n", filemode, statbuf.st_size, timestamp, dent->d_name);
        //}
    }

    closedir(dir);

    return 0;
}
