#ifndef CYSH_VEC_H
#define CYSH_VEC_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

template <typename T>
class vec {
   public:
    vec() {
        arr = (T *)malloc(sizeof(T));
        cap = 1;
        count = 0;
    }

    ~vec() { free(arr); }

    void push(T el) {
        if (count >= cap) {
            cap *= 2;
            arr = (T *)realloc(arr, cap * sizeof(T));
        }

        arr[count++] = el;
    }

    T &operator[](size_t idx) { return arr[idx]; }
    size_t size() const { return count; }

    const T *begin() const { return arr; }
    const T *end() const { return arr + count; }

   private:
    T *arr;
    size_t cap;
    size_t count;
};

#endif  // CYSH_VEC_H
