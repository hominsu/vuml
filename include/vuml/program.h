//
// Created by Homin Su on 2023/6/28.
//

#ifndef VUML_INCLUDE_VUML_PROGRAM_H_
#define VUML_INCLUDE_VUML_PROGRAM_H_

#include <cstddef>
#include <cstdint>

#include <array>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "traits.h"
#include "utils.h"
#include "vuml.h"

#include <vulkan/vulkan.hpp>

namespace vuml {

template<typename ...Ts>
struct type_list {};

namespace details {

template<typename ...Ts>
::std::array<vk::DescriptorType, sizeof...(Ts)> descriptor_types() {
  return {Ts::descriptor_type...};
}

template<::std::size_t ...N>
::std::array<vk::DescriptorSetLayoutBinding, sizeof...(N)> binding_descriptor_types_impl(
    const ::std::array<vk::DescriptorType, sizeof...(N)> &desc_types,
    ::std::index_sequence<N...>
) {
  return {
      {{
           static_cast<uint32_t>(N),
           desc_types[N],
           1,
           vk::ShaderStageFlagBits::eCompute
       }...}
  };
}

template<::std::size_t N>
::std::array<vk::DescriptorSetLayoutBinding, N> binding_descriptor_types(
    const ::std::array<vk::DescriptorType, N> &desc_types
) {
  return binding_descriptor_types_impl(desc_types, ::std::make_index_sequence<N>());
}

template<::std::size_t Index, typename Tuple>
uint32_t element_byte_offset(const Tuple &tuple) {
  return static_cast<uint32_t>(
      reinterpret_cast<const char *>(&::std::get<Index>(tuple)) - reinterpret_cast<const char *>(&tuple)
  );
}

template<typename Tuple, ::std::size_t ...N>
::std::array<vk::SpecializationMapEntry, sizeof...(N)> specs_to_map_entries_impl(
    const Tuple &specs,
    ::std::index_sequence<N...>
) {
  return {
      {{
           static_cast<uint32_t>(N),
           element_byte_offset<N>(specs),
           sizeof(typename ::std::tuple_element_t<N, Tuple>)
       }...}
  };
}

template<typename Tuple>
::std::array<vk::SpecializationMapEntry, traits::tuple_args_len_v<Tuple>> specs_to_map_entries(const Tuple &specs) {
  return specs_to_map_entries_impl(specs, ::std::make_index_sequence<traits::tuple_args_len_v<Tuple>>());
}

template<::std::size_t ...N>
::std::array<vk::WriteDescriptorSet, sizeof...(N)> write_descriptor_set_impl(
    vk::DescriptorSet desc_set,
    const ::std::array<vk::DescriptorBufferInfo, sizeof...(N)> &desc_buf_infos,
    ::std::index_sequence<N...>
) {
  return {
      {{
           desc_set,
           static_cast<uint32_t>(N),
           0,
           1,
           vk::DescriptorType::eStorageBuffer,
           nullptr,
           &desc_buf_infos[N]
       }...}
  };
}

template<::std::size_t N>
::std::array<vk::WriteDescriptorSet, N> write_descriptor_set(
    vk::DescriptorSet desc_set,
    const ::std::array<vk::DescriptorBufferInfo, N> &desc_buf_infos
) {
  return write_descriptor_set_impl(desc_set, desc_buf_infos, ::std::make_index_sequence<N>());
}

struct ComputeBuffer {
 public:
  vk::CommandBuffer cmd_buffer_;
  ::std::unique_ptr<Device, NoopDeleter<Device>> device_;

 public:
  ComputeBuffer(Device &device, vk::CommandBuffer cmd_buffer)
      : cmd_buffer_(cmd_buffer), device_(&device) {}

  void release() noexcept {
    if (device_) {
      device_->freeCommandBuffers(device_->computeCmdPool(), cmd_buffer_);
    }
  }
};

class ProgramBase : NonCopyable {
 protected:
  vk::ShaderModule shader_;
  vk::DescriptorSetLayout desc_layout_;
  vk::DescriptorPool desc_pool_;
  vk::DescriptorSet desc_set_;
  vk::PipelineCache pipe_cache_;
  vk::PipelineLayout pipe_layout_;
  mutable vk::Pipeline pipeline_;
  Device &device_;
  ::std::array<uint32_t, 3> batch_ = {0, 0, 0};

 public:
  void run() {
    auto submit_info = vk::SubmitInfo(0, nullptr, nullptr, 1, &device_.computeCmdBuffer());
    auto queue = device_.computeQueue();
    queue.submit({submit_info}, nullptr);
    queue.waitIdle();
  }

 protected:
  ProgramBase(Device &device, const char *file, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, read_spirv(file), flags) {
  }

  ProgramBase(Device &device, const ::std::vector<uint32_t> &spirv, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, spirv.data(), sizeof(uint32_t) * spirv.size(), flags) {
  }

  ProgramBase(Device &device, const uint32_t *spirv, ::std::size_t size, vk::ShaderModuleCreateFlags flags = {})
      : device_(device) {
    shader_ = device.createShaderModule({flags, size, spirv});
  }

  ~ProgramBase() noexcept { release(); }

  ProgramBase(ProgramBase &&other) noexcept
      : shader_(other.shader_),
        desc_layout_(other.desc_layout_),
        desc_pool_(other.desc_pool_),
        desc_set_(other.desc_set_),
        pipe_cache_(other.pipe_cache_),
        pipe_layout_(other.pipe_layout_),
        pipeline_(other.pipeline_),
        device_(other.device_),
        batch_(other.batch_) {
    other.shader_ = nullptr;
  }

  ProgramBase &operator=(ProgramBase &&other) noexcept {
    release();
    shader_ = other.shader_;
    desc_layout_ = other.desc_layout_;
    desc_pool_ = other.desc_pool_;
    desc_set_ = other.desc_set_;
    pipe_cache_ = other.pipe_cache_;
    pipe_layout_ = other.pipe_layout_;
    pipeline_ = other.pipeline_;
    device_ = other.device_;
    batch_ = other.batch_;

    other.shader_ = nullptr;
    return *this;
  }

  void release() {
    device_.destroyShaderModule(shader_);
    device_.destroyDescriptorPool(desc_pool_);
    device_.destroyDescriptorSetLayout(desc_layout_);
    device_.destroyPipelineCache(pipe_cache_);
    device_.destroyPipeline(pipeline_);
    device_.destroyPipelineLayout(pipe_layout_);
  }

  template<::std::size_t N, typename ...Args>
  void init_pipe_layout(const ::std::array<vk::PushConstantRange, N> &range, Args &...) {
    auto desc_types = details::descriptor_types<Args...>();
    auto bindings = details::binding_descriptor_types(desc_types);
    desc_layout_ = device_.createDescriptorSetLayout(
        {
            vk::DescriptorSetLayoutCreateFlags(),
            static_cast<uint32_t>(bindings.size()),
            bindings.data()
        }
    );
    pipe_cache_ = device_.createPipelineCache({});
    pipe_layout_ = device_.createPipelineLayout(
        {
            vk::PipelineLayoutCreateFlags(),
            1,
            &desc_layout_,
            static_cast<uint32_t>(N),
            range.data()
        }
    );
  }

  template<typename ...Args>
  void alloc_descriptor_sets(Args &...) {
    VUML_ASSERT(desc_layout_);
    auto size = vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, sizeof...(Args));
    auto sizes = ::std::array<vk::DescriptorPoolSize, 1>({size});
    desc_pool_ = device_.createDescriptorPool(
        {
            vk::DescriptorPoolCreateFlags(),
            1,
            static_cast<uint32_t>(sizes.size()),
            sizes.data()
        }
    );
    desc_set_ = device_.allocateDescriptorSets({desc_pool_, 1, &desc_layout_})[0];
  }

  template<typename ...Args>
  void command_buffer_begin(Args &...args) {
    VUML_ASSERT(pipeline_);
    constexpr auto n_args = sizeof...(Args);
    auto desc_infos = ::std::array<vk::DescriptorBufferInfo, n_args>{
        {{
             args.buffer(),
             args.offset() * sizeof(typename Args::value_type),
             args.size_bytes()
         }...}
    };
    auto desc_set = write_descriptor_set(desc_set_, desc_infos);
    device_.updateDescriptorSets(desc_set, {});

    auto cmd_buf = device_.computeCmdBuffer();
    auto begin_info = vk::CommandBufferBeginInfo();
    cmd_buf.begin(begin_info);

    cmd_buf.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline_);
    cmd_buf.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipe_layout_, 0, {desc_set_}, {});
  }

  void command_buffer_end() {
    auto cmd_buf = device_.computeCmdBuffer();
    cmd_buf.dispatch(batch_[0], batch_[1], batch_[2]);
    cmd_buf.end();
  }
};

template<typename Specs>
class SpecBase;

template<template<typename ...> typename Specs, typename ...Spec_Ts>
class SpecBase<Specs<Spec_Ts...>> : public ProgramBase {
 protected:
  ::std::tuple<Spec_Ts...> specs_;

 protected:
  SpecBase(Device &device, const char *file, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, read_spirv(file), flags) {
  }

  SpecBase(Device &device, const ::std::vector<uint32_t> &spirv, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, spirv.data(), sizeof(uint32_t) * spirv.size(), flags) {
  }

  SpecBase(Device &device, const uint32_t *spirv, ::std::size_t size, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, spirv, size, flags) {
  }

  void init_pipeline() {
    auto entries = specs_to_map_entries(specs_);
    auto spec_info = vk::SpecializationInfo(
        static_cast<uint32_t>(entries.size()),
        entries.data(),
        sizeof(specs_),
        &specs_
    );
    auto create_info = vk::PipelineShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eCompute,
        shader_,
        "main",
        &spec_info
    );
    pipeline_ = device_.createPipeline(pipe_layout_, pipe_cache_, create_info);
  }
};

template<>
class SpecBase<type_list<>> : public ProgramBase {
 protected:
  SpecBase(Device &device, const char *file, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, read_spirv(file), flags) {
  }

  SpecBase(Device &device, const ::std::vector<uint32_t> &spirv, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, spirv.data(), sizeof(uint32_t) * spirv.size(), flags) {
  }

  SpecBase(Device &device, const uint32_t *spirv, ::std::size_t size, vk::ShaderModuleCreateFlags flags = {})
      : ProgramBase(device, spirv, size, flags) {
  }

  void init_pipeline() {
    auto create_info = vk::PipelineShaderStageCreateInfo(
        vk::PipelineShaderStageCreateFlags(),
        vk::ShaderStageFlagBits::eCompute,
        shader_,
        "main",
        nullptr
    );
    pipeline_ = device_.createPipeline(pipe_layout_, pipe_cache_, create_info);
  }
};

} // namespace details

template<typename Specs = type_list<>, typename Params = type_list<>>
class Program;

template<template<typename ...> typename Specs, typename ...Specs_Ts, typename Params>
class Program<Specs<Specs_Ts...>, Params> : public details::SpecBase<Specs<Specs_Ts...>> {
 private:
  using Base = details::SpecBase<Specs<Specs_Ts...>>;

 public:
  Program(Device &device, const char *file, vk::ShaderModuleCreateFlags flags = {})
      : Base(device, read_spirv(file), flags) {
  }

  Program(Device &device, const ::std::vector<uint32_t> &spirv, vk::ShaderModuleCreateFlags flags = {})
      : Base(device, spirv.data(), sizeof(uint32_t) * spirv.size(), flags) {
  }

  Program(Device &device, const uint32_t *spirv, ::std::size_t size, vk::ShaderModuleCreateFlags flags = {})
      : Base(device, spirv, size, flags) {
  }

  using Base::run;

  Program &grid(uint32_t x, uint32_t y = 1, uint32_t z = 1) {
    Base::batch_ = {x, y, z};
    return *this;
  }

  Program &spec(Specs_Ts ...specs_ts) {
    Base::specs_ = ::std::make_tuple(specs_ts...);
    return *this;
  }

  template<typename ...Args>
  const Program &bind(const Params &params, Args &&...args) {
    if (!Base::pipeline_) { // not bind
      init_pipe_layout(args...);
      Base::alloc_descriptor_sets(args...);
      Base::init_pipeline();
    }
    create_command_buffer(params, args...);
    return *this;
  }

  template<typename ...Args>
  void run(const Params &params, Args &&...args) {
    bind(params, ::std::forward<Args>(args)...);
    Base::run();
  }

  template<typename ...Args>
  void operator()(const Params &params, Args &&...args) {
    bind(params, ::std::forward<Args>(args)...);
    Base::run();
  }

 private:
  template<typename ...Args>
  void init_pipe_layout(Args &...args) {
    auto range = ::std::array<vk::PushConstantRange, 1>{
        {vk::PushConstantRange(vk::ShaderStageFlagBits::eCompute, 0, sizeof(Params))}
    };
    Base::init_pipe_layout(range, args...);
  }

  template<typename ...Args>
  void create_command_buffer(const Params &params, Args &...args) {
    Base::command_buffer_begin(args...);
    Base::device_.computeCmdBuffer().pushConstants(
        Base::pipe_layout_, vk::ShaderStageFlagBits::eCompute, 0, sizeof(params), &params
    );
    Base::command_buffer_end();
  }
};

template<template<typename ...> typename Specs, typename ...Specs_Ts>
class Program<Specs<Specs_Ts...>, type_list<>> : public details::SpecBase<Specs<Specs_Ts...>> {
 private:
  using Base = details::SpecBase<Specs<Specs_Ts...>>;

 public:
  Program(Device &device, const char *file, vk::ShaderModuleCreateFlags flags = {})
      : Base(device, read_spirv(file), flags) {
  }

  Program(Device &device, const ::std::vector<uint32_t> &spirv, vk::ShaderModuleCreateFlags flags = {})
      : Base(device, spirv.data(), sizeof(uint32_t) * spirv.size(), flags) {
  }

  Program(Device &device, const uint32_t *spirv, ::std::size_t size, vk::ShaderModuleCreateFlags flags = {})
      : Base(device, spirv, size, flags) {
  }

  using Base::run;

  Program &grid(uint32_t x, uint32_t y = 1, uint32_t z = 1) {
    Base::batch_ = {x, y, z};
    return *this;
  }

  Program &spec(Specs_Ts ...specs_ts) {
    Base::specs_ = ::std::make_tuple(specs_ts...);
    return *this;
  }

  template<typename ...Args>
  const Program &bind(Args &&...args) {
    if (!Base::pipeline_) { // not bind
      Base::init_pipe_layout(::std::array<vk::PushConstantRange, 0>{{}, args...});
      Base::alloc_descriptor_sets(args...);
      Base::init_pipeline();
    }
    Base::command_buffer_begin(args...);
    Base::command_buffer_end();
    return *this;
  }

  template<typename ...Args>
  void run(Args &&...args) {
    bind(::std::forward<Args>(args)...);
    Base::run();
  }

  template<typename ...Args>
  void operator()(Args &&...args) {
    bind(::std::forward<Args>(args)...);
    Base::run();
  }
};

} // namespace vuml

#endif //VUML_INCLUDE_VUML_PROGRAM_H_
