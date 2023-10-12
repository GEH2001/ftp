/*func test file*/

#include "common.h"

#include "utils.h"

int main() {

    char buf[20] = "ada";
    strcat(buf, "\012\015");
    printf("%s", buf);
    return 0;
}
 