//
// Created by Homin Su on 2023/5/1.
//

#ifndef VUML_INCLUDE_VUML_ARRAY_DEVICE_ARRAY_H_
#define VUML_INCLUDE_VUML_ARRAY_DEVICE_ARRAY_H_

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <iterator>
#include <type_traits>

#include "alloc_device.h"
#include "basic_array.h"
#include "host_array.h"
#include "iter.h"
#include "properties.h"
#include "vuml/device.h"
#include "vuml/traits.h"
#include "vuml/utils.h"

namespace vuml::array {

template<typename T, class Alloc>
class DeviceOnlyArray : public BasicArray<Alloc> {
 public:
  using value_type = T;

  DeviceOnlyArray(Device &device,
                  ::std::size_t element_nums,
                  vk::MemoryPropertyFlags memory_flags = {},
                  vk::BufferUsageFlags buffer_flags = {})
      : BasicArray<Alloc>(device, element_nums * sizeof(value_type), memory_flags, buffer_flags),
        size_(element_nums) {
  }

  [[nodiscard]] uint32_t size_bytes() const { return size_ * sizeof(value_type); }

 private:
  ::std::size_t size_;
};

template<typename T, class Alloc>
class DeviceArray : public BasicArray<Alloc> {
 public:
  using value_type = T;

 private:
  using Base = BasicArray<Alloc>;

  ::std::size_t size_;

 public:
  DeviceArray(Device &device,
              ::std::size_t element_nums,
              vk::MemoryPropertyFlags memory_flags = {},
              vk::BufferUsageFlags buffer_flags = {})
      : BasicArray<Alloc>(device, element_nums * sizeof(value_type), memory_flags, buffer_flags),
        size_(element_nums) {
  }

  template<typename C, class = typename ::std::enable_if_t<traits::is_iterable_v<C>>>
  DeviceArray(Device &device,
              const C &c,
              vk::MemoryPropertyFlags memory_flags = {},
              vk::BufferUsageFlags buffer_flags = {})
      : DeviceArray(device, c.size(), memory_flags, buffer_flags) {
    fromHost(c.begin(), c.end());
  }

  template<typename It, class = typename ::std::enable_if_t<traits::is_iterator_v<It>>>
  DeviceArray(Device &device,
              It begin,
              It end,
              vk::MemoryPropertyFlags memory_flags = {},
              vk::BufferUsageFlags buffer_flags = {})
      : DeviceArray(device, ::std::distance(begin, end), memory_flags, buffer_flags) {
    fromHost(begin, end);
  }

  template<typename F, class = typename ::std::enable_if_t<::std::is_invocable_v<F, ::std::size_t>>>
  DeviceArray(Device &device,
              ::std::size_t element_nums,
              F &&func,
              vk::MemoryPropertyFlags memory_flags = {},
              vk::BufferUsageFlags buffer_flags = {})
      : DeviceArray(device, element_nums, memory_flags, buffer_flags) {
    auto stage_buf = HostArray<value_type, AllocDevice<properties::HostCoherent>>(Base::_dev, element_nums);
    auto stage_iter = stage_buf.begin();
    for (::std::size_t i = 0; i < element_nums; ++i, ++stage_iter) {
      *stage_iter = func(i);
    }
    copy_buf(Base::device_, stage_buf, *this, size_bytes());
  }

  template<typename It, class = typename ::std::enable_if_t<traits::is_iterator_v<It>>>
  void fromHost(It begin, It end) {
    if (Base::isHostVisible()) {
      ::std::copy(begin, end, host_data());
      Base::device_.unmapMemory(Base::mem_);
    } else {
      auto stage_buf = HostArray<value_type, AllocDevice<properties::HostCoherent>>(Base::device_, begin, end);
      copy_buf(Base::device_, stage_buf, *this, size_bytes());
    }
  }

  template<typename It, class = typename ::std::enable_if_t<traits::is_iterator_v<It>>>
  void fromHost(It begin, It end, ::std::size_t offset) {
    if (Base::isHostVisible()) {
      ::std::copy(begin, end, host_data());
      Base::device_.unmapMemory(Base::mem_);
    } else {
      auto stage_buf = HostArray<value_type, AllocDevice<properties::HostCoherent>>(Base::device_, begin, end);
      copy_buf(Base::device_, stage_buf, *this, size_bytes(), 0u, offset * sizeof(value_type));
    }
  }

  template<typename It, class = typename ::std::enable_if_t<traits::is_iterator_v<It>>>
  void toHost(It dst) const {
    if (Base::isHostVisible()) {
      ::std::copy_n(host_data(), size(), dst);
      Base::device_.unmapMemory(Base::mem_);
    } else {
      auto stage_buf = HostArray<value_type, AllocDevice<properties::HostCached>>(Base::device_, size());
      copy_buf(Base::device_, *this, stage_buf, size_bytes());
      ::std::copy(stage_buf.begin(), stage_buf.end(), dst);
    }
  }

  template<typename It, typename F, class = typename ::std::enable_if_t<
      traits::is_iterator_v<It> && ::std::is_invocable_v<F, value_type>
  >>
  void toHost(It dst, F &&func) const {
    if (Base::isHostVisible()) {
      auto src = host_data();
      ::std::transform(src, src + size(), dst, ::std::forward<F>(func));
      Base::device_.unmapMemory(Base::mem_);
    } else {
      auto stage_buf = HostArray<value_type, AllocDevice<properties::HostCached>>(Base::device_, size());
      copy_buf(Base::device_, *this, stage_buf, size_bytes());
      ::std::transform(stage_buf.begin(), stage_buf.end(), dst, ::std::forward<F>(func));
    }
  }

  template<typename It, typename F, class = typename ::std::enable_if_t<
      traits::is_iterator_v<It> && ::std::is_invocable_v<F, value_type>
  >>
  void toHost(It dst, ::std::size_t size, F &&func) const {
    if (Base::isHostVisible()) {
      auto src = host_data();
      ::std::transform(src, src + size, dst, ::std::forward<F>(func));
      Base::device_.unmapMemory(Base::mem_);
    } else {
      auto stage_buf = HostArray<value_type, AllocDevice<properties::HostCached>>(Base::device_, size);
      copy_buf(Base::device_, *this, stage_buf, size_bytes());
      ::std::transform(stage_buf.begin(), stage_buf.end(), dst, ::std::forward<F>(func));
    }
  }

  template<typename C, class = typename ::std::enable_if_t<traits::is_iterable_v<C>>>
  C toHost() const {
    auto ret = C(size());
    toHost(ret.begin());
    return ret;
  }

  template<typename It>
  void rangeToHost(::std::size_t offset_begin, ::std::size_t offset_end, It dst) const {
    VUML_ASSERT(offset_begin >= 0 && offset_begin < offset_end);
    if (Base::isHostVisible()) {
      auto src = host_data();
      ::std::copy(src + offset_begin, src + offset_end, dst);
      Base::device_.unmapMemory(Base::mem_);
    } else {
      auto stage_buf = HostArray<value_type, AllocDevice<properties::HostCached>>(
          Base::device_, offset_end - offset_begin
      );
      copy_buf(Base::device_, *this, stage_buf, size_bytes(), offset_begin, 0U);
      ::std::copy(stage_buf.begin(), stage_buf.end(), dst);
    }
  }

  [[nodiscard]] uint32_t size() const { return size_; }
  [[nodiscard]] uint32_t size_bytes() const { return size_ * sizeof(value_type); }
  ArrayIter<DeviceArray> device_begin() { return ArrayIter<DeviceArray>(*this, 0); }
  ArrayIter<DeviceArray> device_begin() const { return ArrayIter<DeviceArray>(*this, 0); }
  friend ArrayIter<DeviceArray> device_begin(DeviceArray &array) { return array.device_begin(); }
  ArrayIter<DeviceArray> device_end() { return ArrayIter<DeviceArray>(*this, size_); }
  ArrayIter<DeviceArray> device_end() const { return ArrayIter<DeviceArray>(*this, size_); }
  friend ArrayIter<DeviceArray> device_end(DeviceArray &array) { return array.device_end(); }

 private:
  value_type *host_data() {
    VUML_ASSERT(Base::isHostVisible() && "must host visible");
    return static_cast<value_type *>(Base::device_.mapMemory(Base::mem_, 0, size_bytes()));
  }

  const value_type *host_data() const {
    VUML_ASSERT(Base::isHostVisible() && "must host visible");
    return static_cast<const value_type *>(Base::device_.mapMemory(Base::mem_, 0, size_bytes()));
  }
};

} // namespace vuml::array

#endif //VUML_INCLUDE_VUML_ARRAY_DEVICE_ARRAY_H_
