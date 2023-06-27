//
// Created by Homin Su on 2023/4/28.
//

#include "vuml/device.h"

#include <array>
#include <exception>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "vuml/logger.h"
#include "vuml/traits.h"

namespace {

#ifndef NDEBUG
constexpr ::std::array<const char *, 1> default_extensions = {"VK_KHR_portability_subset"};
#else
constexpr ::std::array<const char *, 1> default_extensions = {"VK_KHR_portability_subset"};
#endif

template<typename T, typename F, class = typename ::std::enable_if_t<
    vuml::traits::is_iterable_v<T> && ::std::is_invocable_v<F, typename T::value_type>
>>
bool contains(const char *s, const T &array, F &&func) {
  return ::std::end(array)
      != ::std::find_if(array.begin(), array.end(), [&](auto &it) -> bool { return 0 == ::std::strcmp(s, func(it)); });
}

template<typename U, typename T, typename F, class = typename ::std::enable_if_t<
    vuml::traits::is_iterable_v<U> && vuml::traits::is_iterable_v<T> && ::std::is_invocable_v<F, typename T::value_type>
>>
::std::vector<const char *> filter_list(::std::vector<const char *> old,
                                        const U &values,
                                        const T &ref_values,
                                        F &&func) {
  for (const auto &v : values) {
    if (contains(v, ref_values, func)) {
      old.push_back(v);
    } else {
      WARN("value %s is missing", v);
    }
  }
  return old;
}

::std::vector<const char *> filter_extensions(const ::std::vector<const char *> &extensions,
                                              const ::std::vector<vk::ExtensionProperties> &avail_extensions) {
  auto r = filter_list({}, extensions, avail_extensions, [&](const auto &property) { return property.extensionName; });
  r = filter_list(::std::move(r),
                  default_extensions,
                  avail_extensions,
                  [&](const auto &property) { return property.extensionName; });
  return r;
}

vk::Device createDevice(const vk::PhysicalDevice &phy_device,
                        uint32_t cmp_family_id,
                        uint32_t tfr_family_id,
                        const ::std::vector<const char *> &extensions) {
  float priority = 1.0;
  auto queue_infos = ::std::array<vk::DeviceQueueCreateInfo, 2>{};
  queue_infos[0] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), cmp_family_id, 1, &priority);
  uint32_t num_queue = 1;
  if (tfr_family_id != cmp_family_id) {
    queue_infos[1] = vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), tfr_family_id, 1, &priority);
    ++num_queue;
  }

  auto ext = filter_extensions(extensions, phy_device.enumerateDeviceExtensionProperties());

  auto device_info = vk::DeviceCreateInfo(vk::DeviceCreateFlags(),
                                          num_queue,
                                          queue_infos.data(),
                                          0,
                                          nullptr,
                                          ext.size(),
                                          ext.data());
  return phy_device.createDevice(device_info);
}

uint32_t getFamilyID(const ::std::vector<vk::QueueFamilyProperties> &queue_families, vk::QueueFlags target_flags) {
  auto ret = static_cast<uint32_t>(-1);
  auto min_flags = ::std::numeric_limits<VkFlags>::max();

  for (uint32_t i = 0; i < queue_families.size(); ++i) {
    auto &q = queue_families[i];
    const auto &flags = q.queueFlags;
    if (q.queueCount > 0 && (target_flags & flags) && VkFlags(flags) < min_flags) {
      ret = i;
      min_flags = VkFlags(flags);
    }
  }
  return ret;
}

vk::CommandBuffer allocCmdBuffer(vk::Device device,
                                 vk::CommandPool pool,
                                 vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary) {
  auto info = vk::CommandBufferAllocateInfo(pool, level, 1);
  return device.allocateCommandBuffers(info)[0];
}

}

namespace vuml {

inline namespace v1 {

Device::Device(Instance &instance, vk::PhysicalDevice &phy_device, const ::std::vector<const char *> &extensions)
    : Device(instance, phy_device, phy_device.getQueueFamilyProperties(), extensions) {
}

Device::~Device() noexcept {
  release();
}

Device::Device(const Device &other)
    : Device(other.instance_, other.phy_device_, other.cmp_family_id_, other.tfr_family_id_, other.extensions_) {
}

Device &Device::operator=(Device other) {
  swap(*this, other);
  return *this;
}

Device::Device(Device &&other) noexcept
    : vk::Device(::std::move(other)),
      instance_(other.instance_),
      phy_device_(other.phy_device_),
      compute_cmd_pool_(other.compute_cmd_pool_),
      compute_cmd_buffer_(other.compute_cmd_buffer_),
      transfer_cmd_pool_(other.transfer_cmd_pool_),
      transfer_cmd_buffer_(other.transfer_cmd_buffer_),
      cmp_family_id_(other.cmp_family_id_),
      tfr_family_id_(other.tfr_family_id_),
      extensions_(::std::move(other.extensions_)) {
}

Device &Device::operator=(Device &&other) noexcept {
  swap(*this, other);
  return *this;
}

void swap(Device &d1, Device &d2) {
  ::std::swap((vk::Device &) d1, (vk::Device &) d2);
  ::std::swap(d1.phy_device_, d2.phy_device_);
  ::std::swap(d1.compute_cmd_pool_, d2.compute_cmd_pool_);
  ::std::swap(d1.compute_cmd_buffer_, d2.compute_cmd_buffer_);
  ::std::swap(d1.transfer_cmd_pool_, d2.transfer_cmd_pool_);
  ::std::swap(d1.transfer_cmd_buffer_, d2.transfer_cmd_buffer_);
  ::std::swap(d1.cmp_family_id_, d2.cmp_family_id_);
  ::std::swap(d1.tfr_family_id_, d2.tfr_family_id_);
  ::std::swap(d1.extensions_, d2.extensions_);
}

vk::PhysicalDeviceProperties Device::properties() const {
  return phy_device_.getProperties();
}

vk::MemoryPropertyFlags Device::memoryProperties(uint32_t id) const {
  return phy_device_.getMemoryProperties().memoryTypes[id].propertyFlags;
}

uint32_t Device::selectMemory(vk::Buffer buffer, vk::MemoryPropertyFlags properties) const {
  auto mem_properties = phy_device_.getMemoryProperties();
  auto mem_requirements = getBufferMemoryRequirements(buffer);
  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
    if ((1u << i) & mem_requirements.memoryTypeBits
        && (properties & mem_properties.memoryTypes[i].propertyFlags) == properties) {
      return i;
    }
  }
  return static_cast<uint32_t>(-1);
}

vk::Queue Device::computeQueue(uint32_t i) {
  return getQueue(cmp_family_id_, i);
}

vk::Queue Device::transferQueue(uint32_t i) {
  return getQueue(tfr_family_id_, i);
}

vk::DeviceMemory Device::alloc(vk::Buffer buffer, uint32_t memory_id) {
  auto mem_requirements = getBufferMemoryRequirements(buffer);
  auto info = vk::MemoryAllocateInfo(mem_requirements.size, memory_id);
  return allocateMemory(info);
}

vk::Pipeline Device::createPipeline(vk::PipelineLayout pipeline_layout,
                                    vk::PipelineCache pipeline_cache,
                                    const vk::PipelineShaderStageCreateInfo &shader_stage_info,
                                    vk::PipelineCreateFlags flags) {
  auto info = vk::ComputePipelineCreateInfo(flags, shader_stage_info, pipeline_layout);
  auto result = createComputePipeline(pipeline_cache, info);
  if (result.result != vk::Result::eSuccess) {
    ERROR("create compute pipeline failed");
    throw ::std::runtime_error("create compute pipeline failed");
  }
  return result.value;
}

vk::CommandBuffer Device::releaseComputeCmdBuffer() {
  auto new_buffer = allocCmdBuffer(*this, compute_cmd_pool_);
  ::std::swap(new_buffer, compute_cmd_buffer_);
  if (cmp_family_id_ == tfr_family_id_) {
    transfer_cmd_buffer_ = compute_cmd_buffer_;
  }
  return new_buffer;
}

Device::Device(Instance &instance,
               vk::PhysicalDevice phy_device,
               const ::std::vector<vk::QueueFamilyProperties> &families,
               const ::std::vector<const char *> &extensions)
    : Device(instance,
             phy_device,
             getFamilyID(families, vk::QueueFlagBits::eCompute),
             getFamilyID(families, vk::QueueFlagBits::eTransfer),
             extensions) {
}

Device::Device(Instance &instance,
               vk::PhysicalDevice phy_device,
               uint32_t cmp_family_id,
               uint32_t tfr_family_id,
               const ::std::vector<const char *> &extensions)
    : vk::Device(createDevice(phy_device, cmp_family_id, tfr_family_id, extensions)),
      instance_(instance),
      phy_device_(phy_device),
      cmp_family_id_(cmp_family_id),
      tfr_family_id_(tfr_family_id),
      extensions_(extensions) {
  try {
    compute_cmd_pool_ = createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer, cmp_family_id_});
    compute_cmd_buffer_ = allocCmdBuffer(*this, compute_cmd_pool_);
    if (cmp_family_id_ == tfr_family_id_) {
      transfer_cmd_pool_ = compute_cmd_pool_;
      transfer_cmd_buffer_ = compute_cmd_buffer_;
    } else {
      transfer_cmd_pool_ = createCommandPool({vk::CommandPoolCreateFlagBits::eResetCommandBuffer, tfr_family_id_});
      transfer_cmd_buffer_ = allocCmdBuffer(*this, transfer_cmd_pool_);
    }
  } catch (vk::Error &) {
    release();
    throw;
  }
}

void Device::release() {
  if (static_cast<vk::Device &>(*this)) {
    if (tfr_family_id_ != cmp_family_id_) {
      freeCommandBuffers(transfer_cmd_pool_, transfer_cmd_buffer_);
      destroyCommandPool(transfer_cmd_pool_);
    }
    freeCommandBuffers(compute_cmd_pool_, compute_cmd_buffer_);
    destroyCommandPool(compute_cmd_pool_);
    vk::Device::destroy();
  }
}

} // namespace v1

} // namespace vuml