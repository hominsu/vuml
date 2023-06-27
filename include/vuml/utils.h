//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_UTILS_H_
#define VUML_INCLUDE_VUML_UTILS_H_

#include <cstddef>
#include <cstdint>

#include <vector>

#include "device.h"

#include <vulkan/vulkan.hpp>

namespace vuml {

::std::vector<uint32_t> read_spirv(const char *filename);

namespace array {

void copy_buf(Device &device,
              vk::Buffer src,
              vk::Buffer dst,
              ::std::size_t size_bytes,
              ::std::size_t src_offset = 0,
              ::std::size_t dst_offset = 0);

} // namespace array
} // namespace vuml

#endif //VUML_INCLUDE_VUML_UTILS_H_
