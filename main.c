/*func test file*/

#include "common.h"

int main() {
    
    char a[10];
    fgets(a, 9, stdin);
    printf("%d\n", a[9]);
    printf("id: %d\n", cmd_to_id(a));
    return 0;
}