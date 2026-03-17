#include <stdio.h>

int main() {
    // oops
    printf("cyShell %d\n", *((int *)NULL));
    return 0;
}
