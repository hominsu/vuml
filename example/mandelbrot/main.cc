//
// Created by Homin Su on 2023/6/28.
//

#include <cstdint>

#include <algorithm>
#include <fstream>

#include "vuml/array.h"
#include "vuml/device.h"
#include "vuml/instance.h"
#include "vuml/logger.h"
#include "vuml/program.h"

void write_ppm(const char *file, const uint32_t *data, uint32_t width, uint32_t height) {
  auto out = ::std::ofstream(file, ::std::ios::binary);
  out << "P6" << "\n" << width << " " << height << " 255" << "\n";
  for (auto i = 0u; i < width * height; ++i) {
    out.put(static_cast<char>(*data++));
    out.put(static_cast<char>(*data++));
    out.put(static_cast<char>(*data++));
    ++data;
  }
}

int main(int argc, char *argv[]) {
  (void) argc, (void) argv;

#ifndef NDEBUG
  vuml::logger::set_log_level(vuml::logger::Level::DEBUG);
#else
  vuml::logger::set_log_level(vuml::logger::Level::INFO);
#endif
  vuml::logger::set_log_file(stdout);

  // [25600, 19200] ~= 1.48G, [3200, 2400] ~= 23M, [320, 240] ~= 233K
  const auto width = 320;
  const auto height = 240;

  auto instance = vuml::Instance();
  auto devices = instance.devices();
  auto discrete_iter = ::std::find_if(
      devices.begin(),
      devices.end(),
      [&](const auto &dev) {
        return dev.properties().deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
      }
  );
  auto dev = discrete_iter != devices.end() ? *discrete_iter : devices.at(0);

  auto mandel = vuml::Array<uint32_t, vuml::memory::Host>(dev, 4 * width * height);

  using Specs = vuml::type_list<uint32_t, uint32_t>;
  struct Params { uint32_t width; uint32_t height; };
  auto program = vuml::Program<Specs, Params>(dev, "shaders/mandelbrot.comp.spv");

  program
      .grid(vuml::div_up(width, 32), vuml::div_up(height, 32))
      .spec(32, 32)({width, height}, mandel);

  write_ppm("mandelbrot.ppm", mandel.data(), width, height);

  return 0;
}
