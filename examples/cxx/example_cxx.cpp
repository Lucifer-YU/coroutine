#include <assert.h>
#include <co.h>
#include <dbg/dbgmsg.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>

// Copy constructor and assignment operator are forbidden.
#ifndef noncopyable
#define noncopyable(name) \
  private:                \
    name(const name &);   \
    name &operator=(const name &);
#endif

#ifdef _WIN32
#ifndef __thread
#define __thread __declspec(thread)
#endif
#endif

namespace co {
class sched {
    noncopyable(sched)

        private : co_sched_t _sched;
    static __thread sched *__tls;

  public:
    sched() {
        assert(!co_sched_self());
        assert(!__tls);
        _sched = co_sched_create();
        __tls = this;
    }
    ~sched() {
        co_sched_destroy(_sched);
        __tls = nullptr;
    }
    static sched *current() {
        return __tls;
    }
    template <typename Fn>
    void go(const Fn &fn, int stackSize = 0) {
        typedef std::function<void()> FnWarp;
        co_sched_create_task(_sched, stackSize,
                             [](void *arg) {
                                 FnWarp *pfn = reinterpret_cast<FnWarp *>(arg);
                                 (*pfn)();
                                 delete pfn;
                             },
                             new FnWarp(fn));	// should stores the wrapper in a list.
    }
    int runloop() {
        return co_sched_runloop(_sched);
    }
};

__thread sched *sched::__tls;

void yield() {
    co_yield();
}
void sleep(uint32_t msec) {
    co_sleep(msec);
}
template <typename Fn>
void go(const Fn &fn, int stackSize = 0) {
    sched *sched = sched::current();
    assert(sched);
    sched->go(fn, stackSize);
}

}; // namespace co

void consumer(int value) {
    co::sleep(10);
    printf("value=%d\n", value);
}

void producer() {
    for (int i = 0; i < 5; i++) {
        if (i % 5 == 0) {
            co::go([i] { consumer(i); });
        }
        co::yield();
    }
}

int main(int argc, char *argv[]) {
    dbg_log_level(DLI_ENTRY);
    co::sched sched;
    sched.go(producer);
    sched.runloop();
    return 0;
}
