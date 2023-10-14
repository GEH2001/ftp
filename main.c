/*func test file*/
#include "utils.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
int main() {
    int ip[6];
    int cd[6];
    int c = sscanf("127,136,15,6,7,44", "%d,%d,%d,%d,%d,%d", &ip[0], &ip[1],&ip[2],&ip[3],&ip[4],&ip[5]);
    memcpy(cd, ip, sizeof cd);
    for(int i = 0; i < 6; i++) {
        printf("%d ", cd[i]);
    }
    printf("\n%ld\n", sizeof cd);
    return 0;
}
