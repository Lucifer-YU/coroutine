#ifndef __COXX_H__
#define __COXX_H__

#include "co.h"
#include <chrono>
#include <stdarg.h>

class Coroutine {
public:
  template <typename Fn, typename ... Args>
  Coroutine(Fn fn, Args&& ... args) {
    return fn(std::forward<Args>(args)...);
  }
};

#endif // __COXX_H__
