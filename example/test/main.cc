#include <cinttypes>

#include <vector>

#include "unistd.h"

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
  vuml::logger::set_log_fd(STDOUT_FILENO);

  auto instance = vuml::Instance();
  auto devices = instance.devices();

  INFO("devices num: %zu", devices.size());
  for (::std::size_t i = 0; i < devices.size(); ++i) {
    INFO("device_%zu name: %s", i, devices[i].properties().deviceName.data());
    INFO("device_%zu shared memory size: %uB", i, devices[i].properties().limits.maxComputeSharedMemorySize);
    INFO("device_%zu work group invocations: %u", i, devices[i].properties().limits.maxComputeWorkGroupInvocations);

    auto wg_count = devices[i].properties().limits.maxComputeWorkGroupCount;
    INFO("device_%zu work group count: [%u,%u,%u]", i, wg_count[0], wg_count[1], wg_count[2]);

    auto wg_size = devices[i].properties().limits.maxComputeWorkGroupSize;
    INFO("device_%zu work group size: [%u,%u,%u]", i, wg_size[0], wg_size[1], wg_size[2]);
  }

  auto &device = devices[0];

  auto x = ::std::vector<float>(64, 1.0f);
  auto d_x = vuml::Array<float, vuml::memory::Host>(device, x.begin(), x.end());
  auto d_y = vuml::Array<float, vuml::memory::HostCached>(device, 64, 2.0f);

  return 0;
}
