#include <stdio.h>

#include "hashmap.h"
#include "str.h"

int main() {
    cyMap<cyString, int, cyStringHash> hashmap;

    hashmap["test"] = 123;

    printf("%d\n", hashmap["test"]);

    return 0;
}
