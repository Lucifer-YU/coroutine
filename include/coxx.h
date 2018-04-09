#ifndef __COXX_H__
#define __COXX_H__

#include "co.h"
#include <chrono>
#include <stdarg.h>
#include <utility> // std::forward

class Scheduler {
};

class Coroutine {
  public:
    template <typename Fn, typename... Args>
    Coroutine(Fn fn, Args &&... args) {
        fn(std::forward<Args>(args)...);
    }
};

#endif // __COXX_H__
