#include <co.h>
#include <dbg/dbgmsg.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

  private:
    co_sched_t _sched;
    static __thread sched* __tls;

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
    static sched* current() {
        return __tls;
     }
    template <typename Fn>
    void go(const Fn &fn, int stackSize = 0) {
        typedef std::function<void()> FnWarp;
        co_sched_create_task(_sched, stackSize,
            [](void *arg) {
                FnWarp* pfn = reinterpret_cast<FnWarp*>(arg);
                (*pfn)();
                delete pfn;
            },
            new FnWarp(fn));
    }
    int runloop() {
        return co_sched_runloop(_sched);
    }
};

__thread sched* sched::__tls;

void yield() {
    co_yield();
}

template<typename Fn>
void go(const Fn &fn, int stackSize = 0) {
    sched* sched = sched::current();
    assert(sched);
    sched->go(fn, stackSize);
}

}; // namespace co

void co_main(int argc, char* argv[]) {
    co::go([argc, argv] {
        printf("task #1 -> argc:%d, argv:%p\n", argc, argv);
        for (int i = 0; i < 10; i++) {
            printf("task #1 -> i = %d\n", i);
            if (i == 3) {
                co::go([] {
                    printf("task #2");
                    for (int j = 0; j < 10; j++) {
                        printf("task #2 -> j = %d\n", j);
                        co::yield();
                    }
                });
            }
            co::yield();
        }
    });
}

int main(int argc, char *argv[]) {
    dbg_log_level(DLI_WARN);
    co::sched sched;
    sched.go([argc, argv] { co_main(argc, argv); });
    sched.runloop();
    return 0;
}
