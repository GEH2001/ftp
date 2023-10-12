/*func test file*/
#include "utils.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
int main() {


    int res = rename(".", "hello");
    if(res != 0) {
        printf("%s(%d)\n", strerror(errno), errno);
    }

    if(access(".gitignore", R_OK) != -1) {
        printf("exist\n");
    } else {
        printf("not exist\n");
    }

    printf("%d\n", is_file_visiable(""));
    printf("%d\n", is_file_visiable("."));
    printf("%d\n", is_file_visiable(".."));
    printf("%d\n", is_file_visiable("main.c"));
    printf("%d\n", is_file_visiable("hello"));
    printf("%d\n", is_file_visiable(".gitignore"));
    printf("%d\n", is_file_visiable("./main.c"));
    printf("%d\n", is_file_visiable("/home/geh"));

    return 0;
}
