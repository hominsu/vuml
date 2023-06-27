//
// Created by Homin Su on 2023/4/28.
//

#include "vuml/instance.h"

#include <array>
#include <type_traits>
#include <utility>

#include "vuml/device.h"
#include "vuml/logger.h"
#include "vuml/traits.h"

#define EXP_ARR(x) static_cast<uint32_t>(x.size()), x.data()

namespace {

#ifndef NDEBUG
constexpr ::std::array<const char *, 1> default_layers = {"VK_LAYER_KHRONOS_validation"};
constexpr ::std::array<const char *, 1> default_extensions = {"VK_KHR_get_physical_device_properties2"};
#else
constexpr ::std::array<const char *, 0> default_layers = {};
constexpr ::std::array<const char *, 1> default_extensions = {"VK_KHR_get_physical_device_properties2"};
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
    if (contains(v, ref_values, ::std::forward<F>(func))) {
      old.push_back(v);
    } else {
      WARN("value %s is missing", v);
    }
  }
  return old;
}

::std::vector<const char *> filter_layers(const ::std::vector<const char *> &layers) {
  auto avail_layers = vk::enumerateInstanceLayerProperties();
  auto r = filter_list({}, layers, avail_layers, [&](const auto &property) { return property.layerName; });
  r = filter_list(::std::move(r),
                  default_layers,
                  avail_layers,
                  [&](const auto &property) { return property.layerName; });
  return r;
}

::std::vector<const char *> filter_extensions(const ::std::vector<const char *> &extensions) {
  auto avail_extensions = vk::enumerateInstanceExtensionProperties();
  auto r = filter_list({}, extensions, avail_extensions, [&](const auto &property) { return property.extensionName; });
  r = filter_list(::std::move(r),
                  default_extensions,
                  avail_extensions,
                  [&](const auto &property) { return property.extensionName; });
  return r;
}

vk::Instance createInstance(::std::vector<const char *> layers,
                            ::std::vector<const char *> extensions,
                            const vk::ApplicationInfo &info) {
  auto create_info = vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &info, EXP_ARR(layers), EXP_ARR(extensions));
  return vk::createInstance(create_info);
}

}

namespace vuml {

inline namespace v1 {

Instance::Instance(const ::std::vector<const char *> &layers,
                   const ::std::vector<const char *> &extensions,
                   const vk::ApplicationInfo &info)
    : instance_(createInstance(filter_layers(layers), filter_extensions(extensions), info)) {
}

Instance::~Instance() noexcept {
  clear();
}

Instance::Instance(Instance &&other) noexcept
    : instance_(other.instance_) {
  other.instance_ = nullptr;
}

Instance &Instance::operator=(Instance &&other) noexcept {
  ::std::swap(instance_, other.instance_);
  return *this;
}

::std::vector<Device> Instance::devices(::std::vector<::std::vector<const char *>> devices_extensions) {
  auto phy_devices = instance_.enumeratePhysicalDevices();
  if (devices_extensions.size() > phy_devices.size()) {
    WARN("the number of devices extensions(%zu) exceed the number of physical devices(%zu)",
         devices_extensions.size(),
         phy_devices.size());
  }
  auto r = ::std::vector<Device>{};
  for (uint32_t i = 0; i < phy_devices.size(); ++i) {
    auto extensions = i + 1 > devices_extensions.size() ? ::std::vector<const char *>{} : devices_extensions[i];
    r.emplace_back(*this, phy_devices[i], extensions);
  }
  return r;
}

void Instance::clear() noexcept {
  if (instance_) {
    instance_.destroy();
    instance_ = nullptr;
  }
}

} // namespace v1

} // namespace vuml