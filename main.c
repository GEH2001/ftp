/*func test file*/

#include "common.h"

int main() {

    command cmd;
    char buf[20];
    fgets(buf, 19, stdin);
    parse_command(buf, &cmd);
    printf("%s(%ld)", cmd.arg, strlen(cmd.arg));
    return 0;
}
