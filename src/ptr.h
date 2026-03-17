#ifndef CYSH_PTR_H
#define CYSH_PTR_H

struct cyCtrlBlock {
    int refcount;
    explicit cyCtrlBlock(int count = 1) : refcount(count) {}
};

template <typename T>
class cyPtr {
   public:
    cyPtr() : ptr(nullptr), ctrl(nullptr) {}

    explicit cyPtr(T *ptr)
        : ptr(ptr), ctrl(ptr ? new cyCtrlBlock() : nullptr) {}

    cyPtr(const cyPtr &other) : ptr(other.ptr), ctrl(other.ctrl) {
        increment();
    }

    cyPtr(cyPtr &&other) : ptr(other.ptr), ctrl(other.ctrl) {
        other.ptr = nullptr;
        other.ctrl = nullptr;
    }

    ~cyPtr() { release(); }

    cyPtr &operator=(const cyPtr &other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            ctrl = other.ctrl;
            increment();
        }
        return *this;
    }

    cyPtr &operator=(cyPtr &&other) noexcept {
        if (this != &other) {
            release();
            ptr = other.ptr;
            ctrl = other.ctrl;
            other.ptr = nullptr;
            other.ctrl = nullptr;
        }
        return *this;
    }

    T *get() const { return ptr; }
    T &operator*() const { return *ptr; }
    T *operator->() const { return ptr; }
    int refcount() const { return ctrl ? ctrl->refcount : 0; }
    explicit operator bool() const { return ptr != nullptr; }

    void reset() {
        release();
        ptr = nullptr;
        ctrl = nullptr;
    }

    void reset(T *ptr) {
        release();
        this->ptr = ptr;
        ctrl = ptr ? new cyCtrlBlock() : nullptr;
    }

   private:
    T *ptr;
    cyCtrlBlock *ctrl;

    void increment() {
        if (ctrl) ++ctrl->refcount;
    }

    void release() {
        if (!ctrl) return;
        if (--ctrl->refcount == 0) {
            delete ptr;
            delete ctrl;
        }
        ptr = nullptr;
        ctrl = nullptr;
    }
};

template <typename T, typename... Args>
cyPtr<T> mkptr(Args &&...args) {
    return cyPtr<T>(new T(static_cast<Args &&>(args)...));
}

#endif  // CYSH_PTR_H
