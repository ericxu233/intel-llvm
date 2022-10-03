//==---------------- pipes.hpp - SYCL pipes ------------*- C++ -*-----------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ===--------------------------------------------------------------------=== //

#pragma once

#include "fpga_utils.hpp"
#include <CL/__spirv/spirv_ops.hpp>
#include <CL/__spirv/spirv_types.hpp>
#include <sycl/ext/oneapi/properties/properties.hpp>
#include <sycl/stl.hpp>
#include <type_traits>

namespace sycl {
__SYCL_INLINE_VER_NAMESPACE(_V1) {
namespace ext::intel::experimental {

template <class Name, class DataT, int32_t MinCapacity = 0,
          class PropertiesT = decltype(oneapi::experimental::properties{}),
          class = void>
class pipe {
  static_assert(std::is_same_v<PropertiesT,
                               decltype(oneapi::experimental::properties{})>,
                "experimental pipe properties are not yet implemented");
};

template <class Name, class DataT, int32_t MinCapacity, class PropertiesT>
class pipe<Name, DataT, MinCapacity, PropertiesT,
           std::enable_if_t<std::is_same_v<
               PropertiesT, decltype(oneapi::experimental::properties{})>>> {
public:
  // Non-blocking pipes
  // Reading from pipe is lowered to SPIR-V instruction OpReadPipe via SPIR-V
  // friendly LLVM IR.
  template <typename FunctionPropertiesT>
  static DataT read(bool &Success, FunctionPropertiesT Properties) {
#ifdef __SYCL_DEVICE_ONLY__
    // Get latency control properties
    using _latency_anchor_id_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_anchor_id_key,
        detail::defaultLatencyAnchorIdProperty>::type;
    using _latency_constraint_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_constraint_key,
        detail::defaultLatencyConstraintProperty>::type;

    // Get latency control property values
    static constexpr int32_t _anchor_id = _latency_anchor_id_prop::value;
    static constexpr int32_t _target_anchor = _latency_constraint_prop::target;
    static constexpr latency_control_type _control_type =
        _latency_constraint_prop::type;
    static constexpr int32_t _relative_cycle = _latency_constraint_prop::cycle;

    int32_t _control_type_code = 0; // latency_control_type::none is default
    if constexpr (_control_type == latency_control_type::exact) {
      _control_type_code = 1;
    } else if constexpr (_control_type == latency_control_type::max) {
      _control_type_code = 2;
    } else if constexpr (_control_type == latency_control_type::min) {
      _control_type_code = 3;
    }

    __ocl_RPipeTy<DataT> _RPipe =
        __spirv_CreatePipeFromPipeStorage_read<DataT>(&m_Storage);
    DataT TempData;
    Success = !static_cast<bool>(__latency_control_nb_read_wrapper(
        _RPipe, &TempData, _anchor_id, _target_anchor, _control_type_code,
        _relative_cycle));
    return TempData;
#else
    (void)Success;
    (void)Properties;
    throw sycl::exception(
        sycl::make_error_code(sycl::errc::feature_not_supported),
        "Pipes are not supported on a host device.");
#endif // __SYCL_DEVICE_ONLY__
  }

  static DataT read(bool &Success) {
    return read(Success, oneapi::experimental::properties{});
  }

  // Writing to pipe is lowered to SPIR-V instruction OpWritePipe via SPIR-V
  // friendly LLVM IR.
  template <typename FunctionPropertiesT>
  static void write(const DataT &Data, bool &Success,
                    FunctionPropertiesT Properties) {
#ifdef __SYCL_DEVICE_ONLY__
    // Get latency control properties
    using _latency_anchor_id_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_anchor_id_key,
        detail::defaultLatencyAnchorIdProperty>::type;
    using _latency_constraint_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_constraint_key,
        detail::defaultLatencyConstraintProperty>::type;

    // Get latency control property values
    static constexpr int32_t _anchor_id = _latency_anchor_id_prop::value;
    static constexpr int32_t _target_anchor = _latency_constraint_prop::target;
    static constexpr latency_control_type _control_type =
        _latency_constraint_prop::type;
    static constexpr int32_t _relative_cycle = _latency_constraint_prop::cycle;

    int32_t _control_type_code = 0; // latency_control_type::none is default
    if constexpr (_control_type == latency_control_type::exact) {
      _control_type_code = 1;
    } else if constexpr (_control_type == latency_control_type::max) {
      _control_type_code = 2;
    } else if constexpr (_control_type == latency_control_type::min) {
      _control_type_code = 3;
    }

    __ocl_WPipeTy<DataT> _WPipe =
        __spirv_CreatePipeFromPipeStorage_write<DataT>(&m_Storage);
    Success = !static_cast<bool>(__latency_control_nb_write_wrapper(
        _WPipe, &Data, _anchor_id, _target_anchor, _control_type_code,
        _relative_cycle));
#else
    (void)Success;
    (void)Data;
    (void)Properties;
    throw sycl::exception(
        sycl::make_error_code(sycl::errc::feature_not_supported),
        "Pipes are not supported on a host device.");
#endif // __SYCL_DEVICE_ONLY__
  }

  static void write(const DataT &Data, bool &Success) {
    write(Data, Success, oneapi::experimental::properties{});
  }

  // Blocking pipes
  // Reading from pipe is lowered to SPIR-V instruction OpReadPipe via SPIR-V
  // friendly LLVM IR.
  template <typename FunctionPropertiesT>
  static DataT read(FunctionPropertiesT Properties) {
#ifdef __SYCL_DEVICE_ONLY__
    // Get latency control properties
    using _latency_anchor_id_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_anchor_id_key,
        detail::defaultLatencyAnchorIdProperty>::type;
    using _latency_constraint_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_constraint_key,
        detail::defaultLatencyConstraintProperty>::type;

    // Get latency control property values
    static constexpr int32_t _anchor_id = _latency_anchor_id_prop::value;
    static constexpr int32_t _target_anchor = _latency_constraint_prop::target;
    static constexpr latency_control_type _control_type =
        _latency_constraint_prop::type;
    static constexpr int32_t _relative_cycle = _latency_constraint_prop::cycle;

    int32_t _control_type_code = 0; // latency_control_type::none is default
    if constexpr (_control_type == latency_control_type::exact) {
      _control_type_code = 1;
    } else if constexpr (_control_type == latency_control_type::max) {
      _control_type_code = 2;
    } else if constexpr (_control_type == latency_control_type::min) {
      _control_type_code = 3;
    }

    __ocl_RPipeTy<DataT> _RPipe =
        __spirv_CreatePipeFromPipeStorage_read<DataT>(&m_Storage);
    DataT TempData;
    __latency_control_bl_read_wrapper(_RPipe, &TempData, _anchor_id,
                                      _target_anchor, _control_type_code,
                                      _relative_cycle);
    return TempData;
#else
    (void)Properties;
    throw sycl::exception(
        sycl::make_error_code(sycl::errc::feature_not_supported),
        "Pipes are not supported on a host device.");
#endif // __SYCL_DEVICE_ONLY__
  }

  static DataT read() { return read(oneapi::experimental::properties{}); }

  // Writing to pipe is lowered to SPIR-V instruction OpWritePipe via SPIR-V
  // friendly LLVM IR.
  template <typename FunctionPropertiesT>
  static void write(const DataT &Data, FunctionPropertiesT Properties) {
#ifdef __SYCL_DEVICE_ONLY__
    // Get latency control properties
    using _latency_anchor_id_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_anchor_id_key,
        detail::defaultLatencyAnchorIdProperty>::type;
    using _latency_constraint_prop = typename detail::GetOrDefaultValT<
        FunctionPropertiesT, latency_constraint_key,
        detail::defaultLatencyConstraintProperty>::type;

    // Get latency control property values
    static constexpr int32_t _anchor_id = _latency_anchor_id_prop::value;
    static constexpr int32_t _target_anchor = _latency_constraint_prop::target;
    static constexpr latency_control_type _control_type =
        _latency_constraint_prop::type;
    static constexpr int32_t _relative_cycle = _latency_constraint_prop::cycle;

    int32_t _control_type_code = 0; // latency_control_type::none is default
    if constexpr (_control_type == latency_control_type::exact) {
      _control_type_code = 1;
    } else if constexpr (_control_type == latency_control_type::max) {
      _control_type_code = 2;
    } else if constexpr (_control_type == latency_control_type::min) {
      _control_type_code = 3;
    }

    __ocl_WPipeTy<DataT> _WPipe =
        __spirv_CreatePipeFromPipeStorage_write<DataT>(&m_Storage);
    __latency_control_bl_write_wrapper(_WPipe, &Data, _anchor_id,
                                       _target_anchor, _control_type_code,
                                       _relative_cycle);
#else
    (void)Data;
    (void)Properties;
    throw sycl::exception(
        sycl::make_error_code(sycl::errc::feature_not_supported),
        "Pipes are not supported on a host device.");
#endif // __SYCL_DEVICE_ONLY__
  }

  static void write(const DataT &Data) {
    write(Data, oneapi::experimental::properties{});
  }

private:
  static constexpr int32_t m_Size = sizeof(DataT);
  static constexpr int32_t m_Alignment = alignof(DataT);
  static constexpr int32_t m_Capacity = MinCapacity;
#ifdef __SYCL_DEVICE_ONLY__
  static constexpr struct ConstantPipeStorage m_Storage = {m_Size, m_Alignment,
                                                           m_Capacity};

  // FPGA BE will recognize this function and extract its arguments.
  // TODO: Pass latency control parameters via the __spirv_* builtin when ready.
  template <typename T>
  static int32_t
  __latency_control_nb_read_wrapper(__ocl_RPipeTy<T> Pipe, T *Data,
                                    int32_t AnchorID, int32_t TargetAnchor,
                                    int32_t Type, int32_t Cycle) {
    return __spirv_ReadPipe(Pipe, Data, m_Size, m_Alignment);
  }

  // FPGA BE will recognize this function and extract its arguments.
  // TODO: Pass latency control parameters via the __spirv_* builtin when ready.
  template <typename T>
  static int32_t
  __latency_control_nb_write_wrapper(__ocl_WPipeTy<T> Pipe, const T *Data,
                                     int32_t AnchorID, int32_t TargetAnchor,
                                     int32_t Type, int32_t Cycle) {
    return __spirv_WritePipe(Pipe, Data, m_Size, m_Alignment);
  }

  // FPGA BE will recognize this function and extract its arguments.
  // TODO: Pass latency control parameters via the __spirv_* builtin when ready.
  template <typename T>
  static void __latency_control_bl_read_wrapper(__ocl_RPipeTy<T> Pipe,
                                                T *Data, int32_t AnchorID,
                                                int32_t TargetAnchor,
                                                int32_t Type, int32_t Cycle) {
    return __spirv_ReadPipeBlockingINTEL(Pipe, Data, m_Size, m_Alignment);
  }

  // FPGA BE will recognize this function and extract its arguments.
  // TODO: Pass latency control parameters via the __spirv_* builtin when ready.
  template <typename T>
  static void
  __latency_control_bl_write_wrapper(__ocl_WPipeTy<T> Pipe, const T *Data,
                                     int32_t AnchorID, int32_t TargetAnchor,
                                     int32_t Type, int32_t Cycle) {
    return __spirv_WritePipeBlockingINTEL(Pipe, Data, m_Size, m_Alignment);
  }
#endif // __SYCL_DEVICE_ONLY__
};

} // namespace ext::intel::experimental
} // __SYCL_INLINE_VER_NAMESPACE(_V1)
} // namespace sycl
