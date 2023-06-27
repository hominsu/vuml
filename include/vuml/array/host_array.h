//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_ARRAY_HOST_ARRAY_H_
#define VUML_INCLUDE_VUML_ARRAY_HOST_ARRAY_H_

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <utility>
#include <type_traits>

#include "basic_array.h"
#include "iter.h"
#include "vuml/device.h"
#include "vuml/traits.h"

namespace vuml::array {

template<typename T, class Alloc>
class HostArray : public BasicArray<Alloc> {
 public:
  using value_type = T;

 private:
  using Base = BasicArray<Alloc>;

  value_type *data_;
  ::std::size_t size_;

 public:
  ~HostArray() noexcept {
    if (data_) { Base::device_.unmapMemory(Base::mem_); }
  }

  HostArray(Device &device,
            ::std::size_t element_nums,
            vk::MemoryPropertyFlags memory_flags = {},
            vk::BufferUsageFlags buffer_flags = {})
      : BasicArray<Alloc>(device, element_nums * sizeof(T), memory_flags, buffer_flags),
        data_(static_cast<value_type *>(Base::device_.mapMemory(Base::mem_, 0, element_nums * sizeof(T)))),
        size_(element_nums) {
  };

  HostArray(Device &device,
            ::std::size_t element_nums,
            value_type value,
            vk::MemoryPropertyFlags memory_flags = {},
            vk::BufferUsageFlags buffer_flags = {})
      : HostArray(device, element_nums, memory_flags, buffer_flags) {
    ::std::fill_n(begin(), element_nums, value);
  };

  template<typename It, class = typename ::std::enable_if_t<traits::is_iterator_v<It>>>
  HostArray(Device &device,
            It begin,
            It end,
            vk::MemoryPropertyFlags memory_flags = {},
            vk::BufferUsageFlags buffer_flags = {})
      : HostArray(device, ::std::distance(begin, end), memory_flags, buffer_flags) {
    ::std::copy(begin, end, this->begin());
  }

  HostArray(HostArray &&other)
      : Base(::std::move(other)), data_(other.data_), size_(other.size_) {
    other.data_ = nullptr;
  }

  HostArray &operator=(HostArray &&other) noexcept {
    this->swap(other);
    return *this;
  }

  void swap(HostArray &other) noexcept {
    ::std::swap(static_cast<Base &>(*this), static_cast<Base &>(other));
    ::std::swap(data_, other.data_);
    ::std::swap(size_, other.size_);
  }

  [[nodiscard]] uint32_t size() const { return size_; }
  [[nodiscard]] uint32_t size_bytes() const { return size_ * sizeof(T); }

  value_type *data() { return data_; }
  const value_type *data() const { return data_; }

  value_type *begin() { return data_; }
  const value_type *begin() const { return data_; }

  value_type *end() { return begin() + size(); }
  const value_type *end() const { return begin() + size(); }

  ArrayIter<HostArray> device_begin() { return ArrayIter<HostArray>(*this, 0); }
  ArrayIter<HostArray> device_begin() const { return ArrayIter<HostArray>(*this, 0); }
  friend ArrayIter<HostArray> device_begin(HostArray &array) { return array.device_begin(); }
  ArrayIter<HostArray> device_end() { return ArrayIter<HostArray>(*this, size_); }
  ArrayIter<HostArray> device_end() const { return ArrayIter<HostArray>(*this, size_); }
  friend ArrayIter<HostArray> device_end(HostArray &array) { return array.device_end(); }

  value_type &operator[](::std::size_t index) { return *(begin() + index); }
  value_type operator[](::std::size_t index) const { return *(begin() + index); }
};

} // namespace vuml::array

#endif //VUML_INCLUDE_VUML_ARRAY_HOST_ARRAY_H_
