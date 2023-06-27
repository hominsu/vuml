//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_ARRAY_BASIC_ARRAY_H_
#define VUML_INCLUDE_VUML_ARRAY_BASIC_ARRAY_H_

#include <cstddef>

#include <exception>
#include <stdexcept>
#include <utility>

#include "alloc_device.h"
#include "vuml/device.h"
#include "vuml/non_copyable.h"
#include "vuml/vuml.h"

#include <vulkan/vulkan.hpp>

namespace vuml::array {

template<class Alloc>
class BasicArray : public vk::Buffer, private NonCopyable {
 protected:
  vk::DeviceMemory mem_;
  vk::MemoryPropertyFlags flags_;
  Device &device_;

 private:
  static constexpr auto descriptor_flag = vk::BufferUsageFlagBits::eStorageBuffer;

 public:
  static constexpr auto descriptor_type = vk::DescriptorType::eStorageBuffer;

  BasicArray(Device &device,
             ::std::size_t size,
             vk::MemoryPropertyFlags properties = {},
             vk::BufferUsageFlags flags = {})
      : vk::Buffer(Alloc::makeBuffer(device, size, descriptor_flag | flags)), device_(device) {
    try {
      auto alloc = Alloc();
      mem_ = alloc.allocMemory(device_, *this, properties);
      flags_ = alloc.memoryProperties(device);
      device_.bindBufferMemory(*this, mem_, 0);
    } catch (::std::runtime_error &) {
      release();
      throw;
    }
  }

  ~BasicArray() noexcept { release(); }

  BasicArray(BasicArray &&other) noexcept
      : vk::Buffer(other), mem_(other.mem_), flags_(other.flags_), device_(other.device_) {
    static_cast<vk::Buffer &>(other) = nullptr;
  }

  vk::Buffer buffer() {
    return *this;
  }

  [[nodiscard]] ::std::size_t offset() const {
    return 0;
  }

  Device &device() {
    return device_;
  }

  [[nodiscard]] bool isHostVisible() const {
    return static_cast<bool>(flags_ & vk::MemoryPropertyFlagBits::eHostVisible);
  }

  BasicArray &operator=(BasicArray &&other) noexcept {
    release();
    mem_ = other.mem_;
    flags_ = other.flags_;
    device_ = other.device_;
    reinterpret_cast<vk::Buffer &>(*this) = reinterpret_cast<vk::Buffer &>(other);
    reinterpret_cast<vk::Buffer &>(other) = nullptr;
    return *this;
  }

  void swap(BasicArray &other) noexcept {
    ::std::swap(static_cast<vk::Buffer &>(*this), static_cast<vk::Buffer &>(other));
    ::std::swap(mem_, other.mem_);
    ::std::swap(flags_, other.flags_);
    ::std::swap(device_, other.device_);
  }

 private:
  void release() noexcept {
    if (static_cast<vk::Buffer &>(*this)) {
      device_.freeMemory(mem_);
      device_.destroyBuffer(*this);
    }
  }
};

} // namespace vuml::array

#endif //VUML_INCLUDE_VUML_ARRAY_BASIC_ARRAY_H_
