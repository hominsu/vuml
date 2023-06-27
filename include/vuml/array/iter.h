//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_ARRAY_ITER_H_
#define VUML_INCLUDE_VUML_ARRAY_ITER_H_

#include <cstddef>
#include <cstdint>

#include <utility>

#include "vuml/device.h"
#include "vuml/vuml.h"

#include <vulkan/vulkan.hpp>

namespace vuml {

template<class Array>
class ArrayIter {
 public:
  using array_type = Array;
  using value_type = typename Array::value_type;

 private:
  array_type *array_;
  ::std::size_t offset_;

 public:
  explicit ArrayIter(array_type &array, ::std::size_t offset)
      : array_(array), offset_(offset) {
  }

  void swap(ArrayIter &other) {
    ::std::swap(array_, other.array_);
    ::std::swap(offset_, other.offset_);
  }

  [[nodiscard]] ::std::size_t offset() const { return offset_; }
  Device &device() { return array_->device(); }
  vk::Buffer &buffer() { return *array_; }
  const array_type &array() const { return *array_; }
  array_type &array() { return *array_; }

  ArrayIter &operator+=(::std::size_t offset) {
    offset_ += offset;
    return *this;
  }

  ArrayIter &operator-=(::std::size_t offset) {
    VUML_ASSERT(offset <= offset_);
    offset_ -= offset;
    return *this;
  }

  bool operator==(const ArrayIter &other) {
    VUML_ASSERT(array_ == other.array_);
    return offset_ == other.offset_;
  }

  bool operator!=(const ArrayIter &other) {
    return *this != other;
  }

  friend ArrayIter operator+(ArrayIter iter, ::std::size_t offset) {
    return iter += offset;
  }

  friend ArrayIter operator-(ArrayIter iter, ::std::size_t offset) {
    return iter -= offset;
  }

  friend ::std::size_t operator-(const ArrayIter &it1, const ArrayIter &it2) {
    VUML_ASSERT(it1.offset() >= it2.offset());
    return it1.offset() - it2.offset();
  }
};

} // namespace vuml

#endif //VUML_INCLUDE_VUML_ARRAY_ITER_H_
