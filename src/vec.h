#ifndef CYSH_VEC_H
#define CYSH_VEC_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

template <typename T>
class cyVec {
   public:
    cyVec(size_t initCap = 1) {
        arr = new T[initCap]();
        cap = initCap;
        count = 0;
    }

    ~cyVec() { delete[] arr; }

    void push(T el) {
        if (count >= cap) {
            T *old = arr;
            cap *= 2;
            arr = new T[cap]();

            for (size_t i = 0; i < count; i++)
                arr[i] = T(static_cast<T &&>(old[i]));

            delete[] old;
        }

        arr[count++] = el;
    }

    T pop() {
        T value = static_cast<T &&>(arr[count - 1]);
        count--;
        return value;
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
