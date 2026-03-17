#ifndef CYSH_STR_H
#define CYSH_STR_H

#include <stddef.h>
#include <string.h>

class cyString {
    char *str;
    size_t len;

   public:
    cyString() : str(nullptr), len(0) {
        str = new char[1];
        str[0] = 0;
    }

    cyString(const char *val) {
        if (val) {
            len = strlen(val);
            str = new char[len + 1];
            strcpy(str, val);
        } else {
            str = new char[1];
            str[0] = 0;
            len = 0;
        }
    }

    cyString(const cyString &src) {
        len = src.len;
        str = new char[len + 1];
        strcpy(str, src.str);
    }

    ~cyString() { delete[] str; }

    bool operator==(const cyString &other) {
        if (other.len != len) return false;
        return strcmp(str, other.str) == 0;
    }

    const char *cstr() const { return str; }
    size_t size() const { return len; }
};

#endif  // CYSH_STR_H
