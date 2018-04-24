#ifndef __COXX_H__
#define __COXX_H__

#include "co.h"
#include <algorithm>
#include <assert.h>
#include <functional>
#include <list>

namespace co {
namespace impl {
#ifdef _WIN32
#define __thread __declspec(thread)
#endif
// Represents a singleton thread local pointer.
template <typename Type>
class tlsptr {
  protected:
    static __thread Type *__tls;
    tlsptr() {
        assert(!__tls);
        __tls = static_cast<Type *>(this);
    }
    ~tlsptr() {
        assert(__tls);
        __tls = nullptr;
    }
    // Non-copyable
    tlsptr(const tlsptr &);
    tlsptr &operator=(const tlsptr &);

  public:
    static Type *tls_get() {
        return __tls;
    }
};
template <typename Type>
__thread Type *tlsptr<Type>::__tls;
} // !namespace impl

// Represents a coroutine scheduler.
class sched : public impl::tlsptr<sched> {
  private:
    co_sched_t _id;

  private:
    struct context {
        sched *_sched;
        std::function<void()> _pfn;
        context(sched *sched, const std::function<void()> &pfn) {
            _sched = sched;
            _pfn = pfn;
        }
    };
    std::list<context *> _ctx_list;

  public:
    sched() {
        assert(!co_sched_self());
        _id = co_sched_create();
    }
    ~sched() {
        co_sched_destroy(_id);
        // Free all remaining contexts (never executed tasks).
        for (auto iter = _ctx_list.begin(); iter != _ctx_list.end(); iter++) {
            delete (*iter);
        }
    }
    template <typename Fn>
    void go(const Fn &fn, int stack_size = 0) {
        context *ctx = new context(this, std::function<void()>(fn));
        _ctx_list.push_back(ctx);
        void *arg = ctx;
        co_sched_create_task(_id, stack_size,
                             [](void *arg) {
                                 context *ctx = reinterpret_cast<context *>(arg);
                                 // We need remove the ctx from ctx-list before all operations (yieldable).
                                 auto iter = std::find(ctx->_sched->_ctx_list.begin(), ctx->_sched->_ctx_list.end(), ctx);
                                 if (iter != ctx->_sched->_ctx_list.end()) {
                                     ctx->_sched->_ctx_list.erase(iter);
                                 }
                                 // Invoke the coroutine function.
                                 ctx->_pfn();
                                 // Delete the ctx now.
                                 delete ctx;
                             },
                             ctx); // should stores the wrapper in a list.
    }
    int runloop() {
        return co_sched_runloop(_id);
    }
};

// Yields the current coroutine task.
inline void yield() {
    co_yield();
}
// 
inline void sleep(uint32_t msec) {
    co_sleep(msec);
}
template <typename Fn>
inline void go(const Fn &fn, int stack_size = 0) {
    assert(co::sched::tls_get());
    co::sched *sched = co::sched::tls_get();
    sched->go(fn, stack_size);
}

}; // namespace co

#endif // __COXX_H__
