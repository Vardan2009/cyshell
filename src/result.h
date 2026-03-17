#ifndef CYSH_RESULT_H
#define CYSH_RESULT_H

#include <assert.h>

template <typename T, typename E>
class cyResult {
   private:
    bool isOk;

    union {
        T value;
        E error;
    };

   public:
    cyResult(const T &v) : isOk(true), value(v) {}
    cyResult(T &&v) : isOk(true), value((T &&)v) {}

    cyResult(const E &e) : isOk(false), error(e) {}
    cyResult(E &&e) : isOk(false), error((E &&)e) {}

    ~cyResult() {
        if (isOk)
            value.~T();
        else
            error.~E();
    }

    bool ok() const { return isOk; }

    T &unwrap() {
        assert(isOk && "tried to unwrap error");
        return value;
    }
    E &unwrapErr() {
        assert(!isOk && "tried to unwrap value");
        return error;
    }
};

template <typename T, typename E>
cyResult<T, E> resOk(T value) {
    return cyResult<T, E>((T &&)value);
}

template <typename T, typename E>
cyResult<T, E> resErr(E error) {
    return cyResult<T, E>((E &&)error);
}

#endif  // CYSH_RESULT_H
