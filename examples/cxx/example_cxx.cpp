#include <co.h>
#include <dbg/dbgmsg.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>

void test1(int a, float b, void *c) {
    printf("test1(a:%d,b:%f,c:%p)\n", a, b, c);
}

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
        assert(!co_sched_ct());
        _sched = co_sched_create();
    }
    ~sched() {
        co_sched_destroy(_sched);
    }
    template <typename Fn>
    void go(const Fn &fn) {
        co_sched_create_task(
            _sched,
            0,
            [](void *arg) {
                const Fn *pfn = (Fn *) arg;
                (*pfn)();
            },
            (void *) &fn);
    }
    int runloop() {
        co_sched_runloop(_sched);
    }
};

void yield() {
    co_yield();
}

}; // namespace co

int co_main(int argc, char* argv[]) {
    /*
    sched.go([&sched, argc, argv] {
        printf("task #1 -> argc:%d, argv:%p\n", argc, argv);
        for (int i = 0; i < 10; i++) {
            printf("task #1 -> i = %d\n", i);
            if (i == 3) {
                sched.go([] {
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
    return 0;
}
int main(int argc, char *argv[]) {
    dbg_log_level(DLI_WARN);
    co::sched sched;
    sched.go(co_main);
    sched.runloop();

    printf("exit......\n");

    return 0;
}
