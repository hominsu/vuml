//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_ARRAY_ALLOC_DEVICE_H_
#define VUML_INCLUDE_VUML_ARRAY_ALLOC_DEVICE_H_

#include <cstddef>
#include <cstdint>

#include <exception>
#include <stdexcept>

#include "vuml/device.h"
#include "vuml/instance.h"
#include "vuml/logger.h"
#include "vuml/vuml.h"

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vuml::array {

template<class Props>
class AllocDevice {
 private:
  uint32_t mem_id_ = -1U;

 public:
  using properties_t = Props;
  using AllocFallback = AllocDevice<typename Props::fallback_t>;

  [[nodiscard]] uint32_t mem_id() const {
    VUML_ASSERT(mem_id_ != -1U);
    return mem_id_;
  }

  [[nodiscard]] vk::MemoryPropertyFlags memoryProperties(Device &device) const {
    return device.memoryProperties(mem_id_);
  }

  static uint32_t findMemory(const Device &device, vk::Buffer buffer, vk::MemoryPropertyFlags flags) {
    auto mem_id = device.selectMemory(
        buffer, static_cast<vk::MemoryPropertyFlags>((static_cast<vk::MemoryPropertyFlags>(Props::memory) | flags))
    );
    if (mem_id != -1U) { return mem_id; }
    WARN("AllocDevice could not find desired memory type, using fallback");
    return AllocFallback::findMemory(device, buffer, flags);
  }

  static vk::Buffer makeBuffer(Device &device, ::std::size_t size, vk::BufferUsageFlags flags) {
    return device.createBuffer({{}, size, flags | vk::BufferUsageFlags(Props::buffer)});
  }

  vk::DeviceMemory allocMemory(Device &device, vk::Buffer buffer, vk::MemoryPropertyFlags flags = {}) {
    mem_id_ = findMemory(device, buffer, flags);
    vk::DeviceMemory mem{};
    try {
      mem = device.allocateMemory({device.getBufferMemoryRequirements(buffer).size, mem_id_});
    } catch (vk::Error &e) {
      auto allocFallback = AllocFallback{};
      WARN("AllocDevice failed to allocate memory, using fallback: %s", e.what());
      mem = allocFallback.allocMemory(device, buffer, flags);
      mem_id_ = allocFallback.mem_id();
    }
    return mem;
  }
};

template<>
class AllocDevice<void> {
 public:
  using properties_t = void;

  [[nodiscard]] uint32_t mem_id() const {
    throw ::std::logic_error("AllocDevice<void>::mem_id() is not supposed to be called");
  }

  [[nodiscard]] vk::MemoryPropertyFlags memoryProperties(Device &device) const {
    (void) device;
    throw ::std::logic_error("AllocDevice<void>::memoryProperties() is not supposed to be called");
  }

  static uint32_t findMemory(const Device &device, vk::Buffer buffer, vk::MemoryPropertyFlags flags) {
    (void) device, (void) buffer, (void) flags;
    throw vk::OutOfDeviceMemoryError(
        "no memory with flags " + std::to_string(static_cast<uint32_t>(flags))
            + " could be found and not fallback available"
    );
  }

  static vk::Buffer makeBuffer(Device &device, ::std::size_t size, vk::BufferUsageFlags flags) {
    return device.createBuffer({{}, size, flags});
  }

  vk::DeviceMemory allocMemory(Device &device, vk::Buffer buffer, vk::MemoryPropertyFlags flags = {}) {
    (void) device, (void) buffer, (void) flags;
    throw vk::OutOfDeviceMemoryError("failed to allocate device memory and no fallback available");
  }
};

} // namespace vuml::array

#endif //VUML_INCLUDE_VUML_ARRAY_ALLOC_DEVICE_H_
