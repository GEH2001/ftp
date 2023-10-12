/*func test file*/

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
int main() {

    int res = mkdir("../hello", 0777);
    char *path = realpath("../hello", NULL);
    printf("%s\n", path);
    if(res == -1) { 
        printf("error(%d)\n", res);
    }
    return 0;
}
 