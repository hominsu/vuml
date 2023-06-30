//
// Created by Homin Su on 2023/6/25.
//

#ifndef VUML_INCLUDE_VUML_UTILS_H_
#define VUML_INCLUDE_VUML_UTILS_H_

#include <cstddef>
#include <cstdint>

#include <utility>
#include <vector>

#include "device.h"
#include "non_copyable.h"

#include <vulkan/vulkan.hpp>

namespace vuml {

inline uint32_t div_up(uint32_t x, uint32_t y) { return (x + y - 1u) / y; }

::std::vector<uint32_t> read_spirv(const char *filename);

namespace array {

void copy_buf(Device &device,
              vk::Buffer src,
              vk::Buffer dst,
              ::std::size_t size_bytes,
              ::std::size_t src_offset = 0,
              ::std::size_t dst_offset = 0);

} // namespace array

template<class T>
struct Resource : public T, private NonCopyable {
  template<typename ...Args>
  explicit Resource(Args &&...args) : T(::std::forward<Args>(args)...) {}

  ~Resource() { T::release(); }

  Resource(Resource &&) noexcept = default;

  Resource &operator=(Resource &&other) noexcept {
    T::release();
    static_cast<T &>(*this) = ::std::move(static_cast<T &>(other));
    return *this;
  }
};

/**
 * @brief to replace default deleter on smart pointer
 * @tparam T
 */
template<class T>
struct NoopDeleter {
  constexpr auto operator()(T *) noexcept -> void {}
};

} // namespace vuml

#endif //VUML_INCLUDE_VUML_UTILS_H_
