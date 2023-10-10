/*func test file*/

#include "common.h"

int main() {

    char buf[200];
    char *a = "../code";
    getcwd(buf, 200);
    printf("%s\n", buf);
    if(chdir(a) == 0) {
        printf("chdir ok\n");
        getcwd(buf, 200);
        printf("%s\n", buf);
    } else {
        printf("chdir fail\n");
    }
    char *b = "./oop";
    chdir(b);
    getcwd(buf, 200);
    printf("%s\n", buf);
    return 0;
}
