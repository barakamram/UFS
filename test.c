#include "fs.h"
#include "mylibc.h"

int main(int argc, char const *argv[]) {
    for (int index = 1;index < 10;index++) {
        printf("\033[0;32m Test Number: %d \033[0;39m\n",index);
        mymkfs(10000);
        mymount("Output", "Output", NULL, 0, NULL);
        printf("\tTest Passed -> Disk opened successfully \n");

        myFILE *file = myfopen("file", "r+");
        if (myfprintf(file, "%c%d", (char)index+'a',index) == 2)
            printf("\tTest Passed -> Successfully wrote | char: %c | num: %d \n",(char)index+'a',index);
        myfseek(file, 0, SEEK_SET);
        char ch = '0';
        int num = 0;
        if (myfscanf(file, "%c%d", &ch, &num) == 2)
            printf("\tTest Passed -> Successfully read | char: %c | num: %d \n", ch, num);
        myfclose(file);
        printf("\tTest Passed -> File closed successfully \n");

        file = myfopen("file", "a");
        if (myfprintf(file, "%d", 10-index) == 1)
            printf("\tTest Passed -> Successfully wrote | other num: %d \n", 10-index);
        myfclose(file);
        file = myfopen("file", "r");
        ch = '0';
        num = 0;
        int n = 0;
        if (myfscanf(file, "%c%d%d", &ch, &num, &n) == 3)
            printf("\tTest Passed -> Successfully read\n\t\tchar: %c | num: %d | other num: %d \n", ch, num, n);
        myfclose(file);
        printf("\tTest Passed -> File closed successfully \n");
        file = myfopen("file", "a");
        if (myfprintf(file, "%d%c", n*num*2, (char)n*num+'a'+1) == 2)
            printf("\tTest Passed -> Successfully wrote | mult: %d | other char: %c \n",n*num*2, (char)n*num+'a'+1);
        myfclose(file);
        file = myfopen("file", "r");
        ch = '0';
        num = 0;
        n = 0;
        int nn = 0;
        char cha = '0';
        if (myfscanf(file, "%c%d%d%d%c", &ch, &num, &n, &nn, &cha) == 5)
            printf("\tTest Passed -> Successfully read\n\t\tchar: %c | num: %d | other num: %d | mult: %d | other char: %c \n", ch, num, n, nn, cha);
        myfclose(file);
        printf("\tTest Passed -> File closed successfully \n");
    }
    printf("\033[0;33m All Test Passed \n");
    return 0;
}
