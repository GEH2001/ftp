/*func test file*/

#include "common.h"

int main() {
    
    char buf[10] = "USER";
    command cmd;
    parse_command(buf, &cmd);
    printf("%s\n", cmd.code);
    return 0;
}
