//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_INSTANCE_H_
#define VUML_INCLUDE_VUML_INSTANCE_H_

#include <vector>

#include "non_copyable.h"

#include <vulkan/vulkan.hpp>

namespace vuml {

inline namespace v1 {

class Device;

class Instance : NonCopyable {
 private:
  vk::Instance instance_;

 public:
  explicit Instance(const ::std::vector<const char *> &layers = {},
                    const ::std::vector<const char *> &extensions = {},
                    const vk::ApplicationInfo &info = {nullptr, 0, nullptr, 0, VK_API_VERSION_1_0});
  ~Instance() noexcept;
  Instance(Instance &&) noexcept;
  Instance &operator=(Instance &&) noexcept;

  ::std::vector<Device> devices(::std::vector<::std::vector<const char *>> devices_extensions = {});

 private:
  void clear() noexcept;
};

} // namespace v1

} // namespace vuml

#endif //VUML_INCLUDE_VUML_INSTANCE_H_
