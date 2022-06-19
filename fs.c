/* credit: https://www.youtube.com/watch?v=n2AAhiujAqs */

#include "fs.h"
#define O_CREAT 0100

// find empty block
int find_empty_block() {
    for (int i = 0; i < sb.num_blocks; i++) {
        if (db[i].next_block_num == -1)
            return i;
    }
    return -1;
}
// find empty inode
int find_empty_inode() {
    for (int i = 0; i < sb.num_inodes; i++) {
        if (inodes[i].first_block == -1)
            return i;
    }
    return -1;
}
// resize file
void shorten_file(int bn) {
    int nn = db[bn].next_block_num;
    if (nn >= 0)
        shorten_file(nn);
    db[bn].next_block_num = -1;
}
int get_block_num(int file, int offeset) {
    int bn = inodes[file].first_block;
    for (int togo = offeset; togo > 0; togo--) {
        bn = db[bn].next_block_num;
    }
    return bn;
}
void set_filesize(int filenum, int size)
{
    int temp = size + BLOCKSIZE - 1;
    int num = temp / BLOCKSIZE;
    int bn = inodes[filenum].first_block;
    // grow the file if necessary
    for (num--; num > 0; num--)
    {
        // check next block number
        int next_num = db[bn].next_block_num;
        if (next_num == -2)
        {
            int empty = find_empty_block();
            db[bn].next_block_num = empty;
            db[empty].next_block_num = -2;
        }
        bn = db[bn].next_block_num;
    }
    // short the file if necessary
    shorten_file(bn);
    db[bn].next_block_num = -2;
}
int allocate_file(const char *name, int size) {
    int fei = find_empty_inode();
    if (fei == -1) {
        perror("not found empty inode");
        exit(1);
    }
    int feb = find_empty_block();
    if (feb == -1) {
        perror("not found empty block");
        exit(1);
    }
    inodes[fei].size = size;
    inodes[fei].first_block = feb;
    db[feb].next_block_num = -2;
    strcpy(inodes[fei].name, name);
    set_filesize(fei, size);
    return fei;
}
// create file
int createf(const char *path, const char *name) {
    int new_fd = allocate_file(name, sizeof(struct mydirent));
    myDIR *dir_fd = myopendir(path);
    struct mydirent *curr_dir = myreaddir(dir_fd);
    curr_dir->fds[curr_dir->size++] = new_fd;
    return new_fd;
}
//write char
void write_c(int fdn, int curr_pos, char data) {
    int pos = curr_pos;
    int index = inodes[fdn].first_block;
    while(pos >= BLOCKSIZE) {
        pos -= BLOCKSIZE;
        if (db[index].next_block_num == -1) {
            perror("error in next_block_num ");
            exit(1);
        } else if (db[index].next_block_num == -2) {
            db[index].next_block_num = find_empty_block();
            index = db[index].next_block_num;
            db[index].next_block_num = -2;
            break;
        } else {
          index = db[index].next_block_num;
        }
    }
    if (curr_pos > inodes[fdn].size)
        inodes[fdn].size = curr_pos + 1;
    db[index].data[pos] = data;
}
// read char
char read_c(int fdn, int curr_pos) {
    int index = inodes[fdn].first_block;
    while(curr_pos >= BLOCKSIZE) {
        curr_pos -= BLOCKSIZE;
        index = db[index].next_block_num;
        if (index == -1 || index == -2){
            perror("EMPTY BLOCK");
            exit(1);
        }
    }
    return db[index].data[curr_pos];
}
// write bytes
void write_b(int fn, int pos, char *data) {
    int relative_block = pos / BLOCKSIZE;
    int bn = get_block_num(fn, relative_block);
    int offset = pos % BLOCKSIZE;
    strcpy(&db[bn].data[offset], data);
}

void mymkfs(int size) {
    int s = size - sizeof(struct super_block);

    sb.num_inodes = (s / 10) / (sizeof(struct inode));
    sb.num_blocks = (s - sb.num_inodes) / (sizeof(struct disk_block));
    sb.size_blocks = BLOCKSIZE;
    inodes = malloc(sizeof(struct inode) * sb.num_inodes);
    for (int i = 0; i < sb.num_inodes; i++) {
        inodes[i].size = -1;
        inodes[i].first_block = -1;
        inodes[i].count = 0;
        strcpy(inodes[i].name, "");
    }
    db = malloc(sizeof(struct disk_block) * sb.num_blocks);
    for (int i = 0; i < sb.num_blocks; i++) {
        db[i].next_block_num = -1;
    }
    int root = allocate_file("root", sizeof(struct mydirent));
    inodes[root].dir = 1;
    struct mydirent *rootdir = malloc(sizeof(struct mydirent));
    for (int i = 0; i < 10; i++) {
        rootdir->fds[i] = -1;
    }
    strcpy(rootdir->d_name, "root");
    rootdir->size = 0;
    char *rootdiraschar = (char *)rootdir;
    for (int i = 0; i < sizeof(struct mydirent); i++) {
        write_c(root, i, rootdiraschar[i]);
    }
    free(rootdir);
}
int mymount(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags, const void *data) {
    if (source == NULL && target == NULL) {
        perror("both source and target are NULL");
        exit(1);
    }
    if (target != NULL) {
      FILE *file = fopen(target, "w+");
      fwrite(&sb, sizeof(struct super_block), 1, file); // super block
      fwrite(inodes, sizeof(struct inode), sb.num_inodes, file); // inodes
      fwrite(db, sizeof(struct disk_block), sb.num_blocks, file); // disk_block
      fclose(file);
    }
    if (source != NULL) {
      FILE *file = fopen(source, "r");
      fread(&sb, sizeof(struct super_block), 1, file); // super_block
      inodes = malloc(sizeof(struct inode) * sb.num_inodes);
      db = malloc(sizeof(struct disk_block) * sb.num_blocks);
      fread(inodes, sizeof(struct inode), sb.num_inodes, file); // inodes
      fread(db, sizeof(struct disk_block), sb.num_blocks, file); // disk_block
      fclose(file);
    }
}
int myopen(const char *pathname, int flags) {
    for (int a = 0; a < sb.num_inodes; a++) {
        if (!strcmp(inodes[a].name, pathname)) {
            if (inodes[a].dir == 1) {
                perror("inodes[a].dir == 1");
                exit(1);
            }
            my_open_file[a].fd = a;
            my_open_file[a].pos = 0;
            return a;
        }
    }
    int b = createf("root", pathname);
    my_open_file[b].fd = b;
    my_open_file[b].pos = 0;
    return b;
}
int myclose(int myfd) {
    my_open_file[myfd].fd = -1;
    my_open_file[myfd].pos = -1;
    return 0;
}
ssize_t myread(int myfd, void *buf, size_t count) {
    if (my_open_file[myfd].fd != myfd || inodes[myfd].dir == 1) {
        perror("EMPTY DIR");
        exit(1);
    }
    char *c = malloc(count + 1);
    for (int i = 0; i < count; i++) {
        c[i] = read_c(myfd, my_open_file[myfd].pos);
        my_open_file[myfd].pos++;
    }
    c[count] = '\0';
    strncpy(buf, c, count);
    free(c);
    return my_open_file[myfd].pos;
}
ssize_t mywrite(int myfd, const void *buf, size_t count) {
    if (my_open_file[myfd].fd != myfd || inodes[myfd].dir == 1) {
        perror("EMPTY DIR");
        exit(1);
    }
    char *c = (char *)buf;
    for (int i = 0; i < count; i++) {
        inodes[myfd].count++;
        write_c(myfd, my_open_file[myfd].pos, c[i]);
        my_open_file[myfd].pos++;
    }
    return my_open_file[myfd].pos;
}
off_t mylseek(int myfd, off_t offset, int whence) {
    if (my_open_file[myfd].fd != myfd) {
        perror("this is not the current file");
        exit(1);
    }
    if (whence == SEEK_CUR)
        my_open_file[myfd].pos += offset;
    else if (whence == SEEK_END)
        my_open_file[myfd].pos += inodes[myfd].count;
    else if (whence == SEEK_SET)
        my_open_file[myfd].pos = offset;
    if (my_open_file[myfd].pos < 0)
        my_open_file[myfd].pos = 0;
    return my_open_file[myfd].pos;
}
myDIR *myopendir(const char *name) {
    char str[100];
    strcpy(str, name);
    char *t;
    const char c[2] = "/";
    t = strtok(str, c);
    char curr[NAME_SIZE] = "";
    char last[NAME_SIZE] = "";
    while (t != NULL) {
        strcpy(last, curr);
        strcpy(curr, t);
        t = strtok(NULL, c);
    }
    for (int i = 0; i < sb.num_inodes; i++) {
        if (!strcmp(inodes[i].name, curr)) {
            if (inodes[i].dir != 1) {
                printf("%s\n", inodes[i].name);
                perror("EMPTY DIR");
                exit(1);
            }
            myDIR *res = (myDIR *)malloc(sizeof(myDIR));
            res->d = i;
            return res;
        }
    }
    int myfd = myopendir(last)->d;
    if (myfd == -1) {
        perror("error");
        exit(1);
    }
    if (inodes[myfd].dir == 0) {
        perror("EMPTY DIR");
        exit(1);
    }
    int diskblock = inodes[myfd].first_block;
    struct mydirent *curr_dir = (struct mydirent *)db[diskblock].data;
    if (curr_dir->size >= 10) {
        perror("size error");
        exit(1);
    }
    int dirfd = allocate_file(curr, sizeof(struct mydirent));
    curr_dir->fds[curr_dir->size++] = dirfd;
    inodes[dirfd].dir = 1;
    struct mydirent *new_dir = malloc(sizeof(struct mydirent));
    new_dir->size = 0;
    for (int i = 0; i < 10; i++) {
        new_dir->fds[i] = -1;
    }
    char *newdir = (char *)new_dir;
    for (size_t i = 0; i < sizeof(struct mydirent); i++) {
        write_b(dirfd, i, &newdir[i]);
    }
    strcpy(new_dir->d_name, curr);
    myDIR *r = (myDIR *)malloc(sizeof(myDIR));
    r->d = dirfd;
    return r;
}
struct mydirent *myreaddir(myDIR *dirp) {
    if (inodes[dirp->d].dir == 0) {
        perror("cannot read dir");
        exit(1);
    }
    return (struct mydirent *)db[inodes[dirp->d].first_block].data;
}
int myclosedir(myDIR *dirp) {
    return 0;
}

// print data
void print_fs() {
    printf("---Super Block Info---\n");
    printf("\tnum_inodes %d\n", sb.num_inodes);
    printf("\tnum_blocks %d\n", sb.num_blocks);
    printf("\tsize_blocks %d\n", sb.size_blocks);

    printf("Inodes:\n");
    for (int i = 0; i < sb.num_inodes; i++){
        printf("\tname: %s size: %d block: %d\n", inodes[i].name, inodes[i].size, inodes[i].first_block);
    }

    printf("Block:\n");
    for (int i = 0; i < sb.num_blocks; i++) {
        printf("\tblock num: %d next block %d\n\n", i, db[i].next_block_num);
    }
}
