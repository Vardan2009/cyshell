#include <stdio.h>

#include "hashmap.h"

int main() {
    cyMap<const char *, int, cyStringHash> hashmap;

    hashmap["test"] = 123;

    printf("%d\n", hashmap["test"]);

    return 0;
}
