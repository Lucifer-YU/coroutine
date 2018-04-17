#include <co.h>
#include <dbg/dbgmsg.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Copy constructor and assignment operator are forbidden.
#define NONCOPYABLE(name) \
  private:                \
    name(const name &);   \
    name &operator=(const name &);

namespace co {
class sched {
    NONCOPYABLE(sched)

  private:
    co_sched_t _sched;

  public:
    sched() {
        assert(!co_sched_self());
        _sched = co_sched_create();
    }
    ~sched() {
        co_sched_destroy(_sched);
    }
    template <typename Fn>
    void go(const Fn &fn, int stackSize = 0) {
        co_sched_create_task(_sched, stackSize,
            [](void *arg) {
                const Fn *pfn = (Fn *) arg;
                (*pfn)();
            },
            (void *) &fn);
    }
    int runloop() {
        return co_sched_runloop(_sched);
    }
};

void yield() {
    co_yield();
}
template<typename Fn>
void go(const Fn &fn, int stackSize = 0) {
    co_sched_t sched = co_sched_self();
    assert(sched);
    co_sched_create_task(sched, stackSize,
        [](void *arg) {
            const Fn *pfn = (Fn *) arg;
            (*pfn)();
        },
        (void *) &fn);
}
template<typename Fn, typename ... Args>
void go1(const Fn& fn, Args ... args) {
    co_sched_t sched = co_sched_self();
    assert(sched);
    co_sched_create_task(sched, 0,
        [&args](void *arg) {
            const Fn *pfn = (Fn *) arg;
            (*pfn)(std::forward<Args>(args)...);
        },
        (void *) &fn);
}

}; // namespace co

void a(int x, float y, void* z) {
    printf("a(x:%d,y:%f,z:%p)\n", x, y, z);
}

void co_main(int argc, char* argv[]) {
    printf("hello, world!\n");
    co::go1(a, 1, 1.0, (void*)NULL);
    /*
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
    */
}

int main(int argc, char *argv[]) {
    dbg_log_level(DLI_WARN);
    co::sched sched;
    sched.go([argc, argv] { co_main(argc, argv); });
    sched.runloop();
    return 0;
}
