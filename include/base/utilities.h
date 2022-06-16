#ifndef UTILITIES
#define UTILITIES

#include "spdlog/spdlog.h"
#include <unordered_set>
#include <vector>

void osSignalFailureHandler(int sig);

void AddSegfaultHandler();

template <class B>
class PtrVec {
public:
  template <class D>
  D* Add(D&& t) {
    static_assert(std::is_convertible<D*, B*>::value, "D must inherit B (Add<D> for PtrVec<B>) to use this PtrVec");
    auto pt = new D(t);
    values.push_back(pt);
    return pt;
  }

  const std::vector<B*>& Get() const { return values; }
  const std::vector<B*>* operator->() const { return &values; }
  B* operator[](int idx) const { return values[idx]; }

  void Remove(int idx) {
    auto iter = values.begin() + idx;
    delete *iter;
    values.erase(iter);
  }

  ~PtrVec() {
    for (auto v : values)
      delete v;
  }

private:
  std::vector<B*> values;
};

template <class B>
class PtrSet {
public:
  template <class D>
  D* Add(D&& t) {
    static_assert(std::is_convertible<D*, B*>::value, "D must inherit B (Add<D> for PtrSet<B>) to use this PtrSet");
    auto pt = new D(t);
    values.emplace(pt);
    return pt;
  }

  const std::unordered_set<B*>& Get() const { return values; }
  const std::unordered_set<B*>* operator->() const { return &values; }

  void Remove(B* t) {
    values.erase(t);
    delete t;
  }

  ~PtrSet() {
    for (auto v : values)
      delete v;
  }

private:
  std::unordered_set<B*> values;
};

#endif