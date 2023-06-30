//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_DEVICE_H_
#define VUML_INCLUDE_VUML_DEVICE_H_

#include <cstdint>

#include <vector>

#include <vulkan/vulkan.hpp>

namespace vuml {

inline namespace v1 {

class Instance;

class Device : public vk::Device {
 private:
  Instance &instance_;
  vk::PhysicalDevice phy_device_;
  vk::CommandPool compute_cmd_pool_;
  vk::CommandBuffer compute_cmd_buffer_;
  vk::CommandPool transfer_cmd_pool_;
  vk::CommandBuffer transfer_cmd_buffer_;
  uint32_t cmp_family_id_ = -1U;
  uint32_t tfr_family_id_ = -1U;
  ::std::vector<const char *> extensions_;

 public:
  explicit Device(Instance &instance, vk::PhysicalDevice &phy_device, const ::std::vector<const char *> &extensions);
  ~Device() noexcept;
  Device(const Device &);
  Device &operator=(Device);
  Device(Device &&) noexcept;
  Device &operator=(Device &&) noexcept;
  friend void swap(Device &, Device &);

  [[nodiscard]] vk::PhysicalDeviceProperties properties() const;
  [[nodiscard]] vk::MemoryPropertyFlags memoryProperties(uint32_t id) const;
  [[nodiscard]] static uint32_t numComputeQueues() { return 1u; }
  [[nodiscard]] static uint32_t numTransferQueues() { return 1u; }
  [[nodiscard]] uint32_t selectMemory(vk::Buffer buffer, vk::MemoryPropertyFlags properties) const;
  Instance &instance() { return instance_; }
  [[nodiscard]] const Instance &instance() const { return instance_; }
  [[nodiscard]] bool hasSeparateQueues() const { return cmp_family_id_ == tfr_family_id_; }

  vk::Queue computeQueue(uint32_t i = 0);
  vk::Queue transferQueue(uint32_t i = 0);
  vk::DeviceMemory alloc(vk::Buffer buffer, uint32_t memory_id);
  vk::CommandPool computeCmdPool() { return compute_cmd_pool_; }
  vk::CommandBuffer &computeCmdBuffer() { return compute_cmd_buffer_; }
  vk::CommandPool transferCmdPool() { return transfer_cmd_pool_; }
  vk::CommandBuffer &transferCmdBuffer() { return transfer_cmd_buffer_; }
  vk::Pipeline createPipeline(vk::PipelineLayout pipeline_layout,
                              vk::PipelineCache pipeline_cache,
                              const vk::PipelineShaderStageCreateInfo &shader_stage_info,
                              vk::PipelineCreateFlags flags = {});
  vk::CommandBuffer releaseComputeCmdBuffer();

 private:
  Device(Instance &instance,
         vk::PhysicalDevice phy_device,
         const std::vector<vk::QueueFamilyProperties> &families,
         const ::std::vector<const char *> &extensions);
  Device(Instance &instance,
         vk::PhysicalDevice phy_device,
         uint32_t cmp_family_id,
         uint32_t tfr_family_id,
         const ::std::vector<const char *> &extensions);
  void release();
};

} // namespace v1

} // namespace vuml

#endif //VUML_INCLUDE_VUML_DEVICE_H_
