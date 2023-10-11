/*func test file*/

#include "common.h"

#include "utils.h"

int main() {

    char buf[BSIZE];
    list_files(buf, BSIZE, "");

    printf("%s", buf);
    printf("%ld\n", strlen(buf));

    return 0;
}
 