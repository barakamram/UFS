/* credit: https://www.youtube.com/watch?v=n2AAhiujAqs */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_MAX 128
#define BLOCKSIZE 512
#define MAX_FILES 10000
#define NAME_SIZE 10

struct super_block {
    int num_inodes;
    int num_blocks;
    int size_blocks;
};

struct inode {
    int size;
    int dir;
    int first_block;
    int count;
    char name[NAME_SIZE];
};

struct disk_block {
    int next_block_num;
    char data[BLOCKSIZE];
};

struct open_file {
    int fd;
    int pos;
};

struct mydirent {
    int size;
    int fds[10];
    char d_name[NAME_SIZE];
};

typedef struct dir {
    int d;
    char *dir_name;
} myDIR;

void mymkfs(int size);
int mymount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data);
int myopen(const char *pathname, int flags);
int myclose(int myfd);
ssize_t myread(int myfd, void *buf, size_t count);
ssize_t mywrite(int myfd, const void *buf, size_t count);
off_t mylseek(int myfd, off_t offset, int whence);
myDIR *myopendir(const char *name);
struct mydirent *myreaddir(myDIR *dirp);
int myclosedir(myDIR *dirp);
void print_fs();

struct super_block sb;
struct inode *inodes;
struct disk_block *db;
struct open_file my_open_file[MAX_FILES];
