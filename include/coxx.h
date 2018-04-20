#ifndef __COXX_H__
#define __COXX_H__

#include "co.h"
#include <assert.h>
#include <functional>

namespace co {
class sched {
private:
    // Non-copyable
    sched(const sched &);
    sched &operator=(const sched &);

private:
    co_sched_t _id;
    bool _shadow = false;

public:
    sched() {
        assert(!co_sched_self());
        _id = co_sched_create();
    }
    sched(co_sched_t id) {
        assert(id);
        _id = id;
        _shadow = true;
    }
    ~sched() {
        if (!_shadow)
            co_sched_destroy(_id);
    }
    static co_sched_t currentId() {
        return co_sched_self();
    }
    template <typename Fn>
    void go(const Fn &fn, int stackSize = 0) {
        typedef std::function<void()> FnWarp;
        co_sched_create_task(_id, stackSize,
                             [](void *arg) {
                                 FnWarp *pfn = reinterpret_cast<FnWarp *>(arg);
                                 (*pfn)();
                                 delete pfn;
                             },
                             new FnWarp(fn)); // should stores the wrapper in a list.
    }
    int runloop() {
        return co_sched_runloop(_id);
    }
};

inline void yield() {
    co_yield();
}
inline void sleep(uint32_t msec) {
    co_sleep(msec);
}
template <typename Fn>
inline void go(const Fn &fn, int stackSize = 0) {
    assert(co::sched::currentId());
    co::sched sched(co::sched::currentId());
    sched.go(fn, stackSize);
}

}; // namespace co

#endif // __COXX_H__
