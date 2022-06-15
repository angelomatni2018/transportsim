#ifndef UTILITIES
#define UTILITIES

#include "spdlog/spdlog.h"
#include <vector>

void osSignalFailureHandler(int sig);

void AddSegfaultHandler();

template <class B>
class Pool {
public:
  template <class D>
  D* With(D&& t) {
    static_assert(std::is_convertible<D*, B*>::value, "D must inherit B (With<D> for Pool<B>) to use this Pool");
    auto pt = new D(t);
    values.push_back(pt);
    return pt;
  }

  ~Pool() {
    for (auto v : values)
      delete v;
  }

private:
  std::vector<B*> values;
};

#endif