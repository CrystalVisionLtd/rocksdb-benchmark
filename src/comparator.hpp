//
// Created by wuyua on 11/23/2021.
//

#ifndef COMPARATOR_HPP
#define COMPARATOR_HPP

#include <rocksdb/comparator.h>

using namespace rocksdb;

class UInt64ComparatorImpl : public Comparator {
 public:
  int Compare(const Slice& a, const Slice& b) const override {
    auto ia = *reinterpret_cast<const uint64_t*>(a.data());
    auto ib = *reinterpret_cast<const uint64_t*>(b.data());
    if (ia < ib) return -1;
    if (ia > ib) return 1;
    return 0;
  }
  const char* Name() const override { return "UInt64Comparator"; }
  void FindShortestSeparator(std::string* start, const Slice& limit)
      const override {}
  void FindShortSuccessor(std::string* key) const override {}
};

const Comparator* UInt64Comparator() {
  static UInt64ComparatorImpl comparator;
  return &comparator;
}
#endif  // COMPARATOR_HPP
