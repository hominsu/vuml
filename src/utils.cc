//
// Created by Homin Su on 2023/4/29.
//

#include "vuml/utils.h"

#include <cstdint>

#include <exception>
#include <fstream>
#include <iterator>
#include <string>

#include "vuml/logger.h"

namespace vuml {
::std::vector<uint32_t> read_spirv(const char *filename) {
  auto f = ::std::ifstream(filename, ::std::ios::binary | ::std::ios::ate);
  if (!f.is_open()) {
    ERROR("open %s failed", filename);
    throw ::std::runtime_error(::std::string("open ") + filename + "failed");
  }
  const auto size = static_cast<uint32_t>(f.tellg());

  auto ret = ::std::vector<uint32_t>((size + 3) / 4);  // align to 4 bytes
  ::std::copy(::std::istreambuf_iterator<char>(f),
              ::std::istreambuf_iterator<char>(),
              reinterpret_cast<char *>(ret.data()));
  return ret;
}

namespace array {

void copy_buf(Device &device,
              vk::Buffer src,
              vk::Buffer dst,
              ::std::size_t size_bytes,
              ::std::size_t src_offset,
              ::std::size_t dst_offset) {
  auto cmd_buffer = device.transferCmdBuffer();
  cmd_buffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  cmd_buffer.copyBuffer(src, dst, vk::BufferCopy(src_offset, dst_offset, size_bytes));
  cmd_buffer.end();
  auto queue = device.transferQueue();
  queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &cmd_buffer), nullptr);
  queue.waitIdle();
}

} // namespace array

} // namespace vuml
