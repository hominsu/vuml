//
// Created by Homin Su on 2023/6/26.
//

#ifndef VUML_INCLUDE_VUML_ARRAY_H_
#define VUML_INCLUDE_VUML_ARRAY_H_

#include "array/alloc_device.h"
#include "array/device_array.h"
#include "array/host_array.h"
#include "array/properties.h"

namespace vuml {
namespace details {
template<class Props>
struct ArrayClass {
  template<class T, class Alloc> using type = array::HostArray<T, Alloc>;
};

template<>
struct ArrayClass<array::properties::Device> {
  template<class T, class Alloc> using type = array::DeviceArray<T, Alloc>;
};

template<>
struct ArrayClass<array::properties::DeviceOnly> {
  template<class T, class Alloc> using type = array::DeviceOnlyArray<T, Alloc>;
};
} // namespace details

namespace memory {
using Host = array::AllocDevice<array::properties::Host>;
using HostCoherent = array::AllocDevice<array::properties::HostCoherent>;
using HostCached = array::AllocDevice<array::properties::HostCached>;
using Unified = array::AllocDevice<array::properties::Unified>;
using Device = array::AllocDevice<array::properties::Device>;
using DeviceOnly = array::AllocDevice<array::properties::DeviceOnly>;
} // namespace memory

template<class T, class Alloc=array::AllocDevice<array::properties::Device>>
using Array = typename details::ArrayClass<typename Alloc::properties_t>::template type<T, Alloc>;

} // namespace vuml

#endif //VUML_INCLUDE_VUML_ARRAY_H_
