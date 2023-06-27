//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_ARRAY_PROPERTIES_H_
#define VUML_INCLUDE_VUML_ARRAY_PROPERTIES_H_

#include <type_traits>

#include <vulkan/vulkan.hpp>

namespace vuml::array::properties {

using memflags_t = ::std::underlying_type_t<vk::MemoryPropertyFlagBits>;
using bufflags_t = ::std::underlying_type_t<vk::BufferUsageFlagBits>;

struct Host {
  using fallback_t = void;
  static constexpr memflags_t memory = static_cast<memflags_t>(vk::MemoryPropertyFlagBits::eHostVisible);
  static constexpr bufflags_t buffer = {};
};

struct HostCoherent {
  using fallback_t = Host;
  static constexpr memflags_t memory = static_cast<memflags_t>(
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
  );
  static constexpr bufflags_t buffer = static_cast<bufflags_t>(vk::BufferUsageFlagBits::eTransferSrc);
};

struct HostCached {
  using fallback_t = Host;
  static constexpr memflags_t memory = static_cast<memflags_t>(
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCached
  );
  static constexpr bufflags_t buffer = static_cast<bufflags_t>(vk::BufferUsageFlagBits::eTransferDst);
};

struct Unified {
  using fallback_t = void;
  static constexpr memflags_t memory = static_cast<memflags_t>(
      vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible
  );
  static constexpr bufflags_t buffer = {};
};

struct Device {
  using fallback_t = Host;
  static constexpr memflags_t memory = static_cast<memflags_t>(vk::MemoryPropertyFlagBits::eDeviceLocal);
  static constexpr bufflags_t buffer = static_cast<bufflags_t>(
      vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst
  );
};

struct DeviceOnly {
  using fallback_t = Host;
  static constexpr memflags_t memory = static_cast<memflags_t>(vk::MemoryPropertyFlagBits::eDeviceLocal);
  static constexpr bufflags_t buffer = {};
};

} // namespace vuml::array::properties

#endif //VUML_INCLUDE_VUML_ARRAY_PROPERTIES_H_
