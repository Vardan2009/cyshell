#ifndef CYSH_HASHMAP_H
#define CYSH_HASHMAP_H

#include <stddef.h>
#include <stdlib.h>

#include "str.h"

template <typename K, typename V, typename Hash>
class cyMap {
    struct node {
        K key;
        V val;
        node *next = nullptr;
        node(const K &k, const V &v) : key(k), val(v) {}
    };

    node **buckets;
    size_t cap;
    size_t count;
    Hash hasher;

    static constexpr float MAX_LOAD = 0.75f;

    size_t bucketIdx(const K &k) const { return hasher(k) % cap; }

    void rehash() {
        size_t oldCap = cap;
        node **oldBuckets = buckets;

        cap *= 2;
        buckets = (node **)calloc(cap, sizeof(node *));
        count = 0;

        for (size_t i = 0; i < oldCap; ++i) {
            node *n = oldBuckets[i];
            while (n) {
                insert(n->key, n->val);
                node *tmp = n->next;
                delete n;
                n = tmp;
            }
        }

        delete[] oldBuckets;
    }

   public:
    explicit cyMap(size_t initCap = 16) : cap(initCap), count(0) {
        buckets = new node *[cap]();
    }

    ~cyMap() {
        clear();
        delete[] buckets;
    }
    void insert(const K &k, const V &v) {
        if ((float)(count + 1) / cap > MAX_LOAD) rehash();

        size_t idx = bucketIdx(k);
        node *n = buckets[idx];

        while (n) {
            if (n->key == k) {
                n->val = v;
                return;
            }
            n = n->next;
        }

        node *newN = new node(k, v);
        newN->next = buckets[idx];
        buckets[idx] = newN;
        ++count;
    }

    V *find(const K &k) const {
        size_t idx = bucketIdx(k);
        node *n = buckets[idx];
        while (n) {
            if (n->key == k) return &n->val;
            n = n->next;
        }
        return nullptr;
    }

    V &operator[](const K &k) {
        if (auto *v = find(k)) return *v;
        insert(k, V{});
        return *find(k);
    }

    bool erase(const K &k) {
        size_t idx = bucketIdx(k);
        node **curr = &buckets[idx];

        while (*curr) {
            if ((*curr)->key == k) {
                node *toDelete = *curr;
                *curr = toDelete->next;
                delete toDelete;
                --count;
                return true;
            }
            curr = &(*curr)->next;
        }
        return false;
    }

    bool contains(const K &k) const { return find(k) != nullptr; }
    size_t size() const { return count; }
    bool empty() const { return count == 0; }

    void clear() {
        for (size_t i = 0; i < cap; ++i) {
            node *n = buckets[i];
            while (n) {
                node *tmp = n->next;
                delete n;
                n = tmp;
            }
            buckets[i] = nullptr;
        }
        count = 0;
    }
};

struct cyIntHash {
    size_t operator()(int key) const {
        size_t x = (size_t)key;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        return (x >> 16) ^ x;
    }
};

struct cyStringHash {
    size_t operator()(const cyString &str) const {
        const char *key = str.cstr();
        size_t hash = 14695981039346656037ULL;
        while (*key) {
            hash ^= static_cast<size_t>(*key++);
            hash *= 1099511628211ULL;
        }
        return hash;
    }
};

struct cyCStringHash {
    size_t operator()(const char *key) const {
        size_t hash = 14695981039346656037ULL;
        while (*key) {
            hash ^= static_cast<size_t>(*key++);
            hash *= 1099511628211ULL;
        }
        return hash;
    }
};

#endif  // CYSH_HASHMAP_H
