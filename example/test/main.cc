#include <algorithm>
#include <vector>

#include "vuml/array.h"
#include "vuml/device.h"
#include "vuml/instance.h"
#include "vuml/logger.h"

int main() {
#ifndef NDEBUG
  vuml::logger::set_log_level(vuml::logger::Level::DEBUG);
#else
  vuml::logger::set_log_level(vuml::logger::Level::INFO);
#endif
  vuml::logger::set_log_file(stdout);

  auto instance = vuml::Instance();
  auto devices = instance.devices();
  auto discrete_iter = ::std::find_if(
      devices.begin(),
      devices.end(),
      [&](const auto &dev) {
        return dev.properties().deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
      }
  );
  auto device = discrete_iter != devices.end() ? *discrete_iter : devices.at(0);

  INFO("devices num: %zu, choose: [%s]", devices.size(), device.properties().deviceName.data());
  for (const auto &dev : devices) {
    INFO("[%s] type: %s",
         dev.properties().deviceName.data(),
         vk::to_string(dev.properties().deviceType).c_str());
    INFO("[%s] shared memory size: %uMB",
         dev.properties().deviceName.data(),
         dev.properties().limits.maxComputeSharedMemorySize / 1024);
    INFO("[%s] work group invocations: %u",
         dev.properties().deviceName.data(),
         dev.properties().limits.maxComputeWorkGroupInvocations);

    auto wg_count = dev.properties().limits.maxComputeWorkGroupCount;
    INFO("[%s] work group count: [%u,%u,%u]",
         dev.properties().deviceName.data(),
         wg_count[0],
         wg_count[1],
         wg_count[2]);

    auto wg_size = dev.properties().limits.maxComputeWorkGroupSize;
    INFO("[%s] work group size: [%u,%u,%u]",
         dev.properties().deviceName.data(),
         wg_size[0],
         wg_size[1],
         wg_size[2]);
  }

  auto x = ::std::vector<float>(64, 1.0f);
  auto d_x = vuml::Array<float, vuml::memory::Host>(device, x.begin(), x.end());
  auto d_y = vuml::Array<float, vuml::memory::HostCached>(device, 64, 2.0f);

  return 0;
}
