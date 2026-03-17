#include <stdio.h>

#include "vec.h"

int main() {
    vec<int> myArr;

    myArr.push(1);
    myArr.push(2);
    myArr.push(3);

    for (int i : myArr) {
        printf("- %d\n", i);
    }

    for (size_t i = 0; i < myArr.size(); ++i) printf("- %d\n", myArr[i]);

    return 0;
}
