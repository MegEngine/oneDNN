/*******************************************************************************
 * Copyright 2020-2022 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *******************************************************************************/
#ifndef BACKEND_GRAPH_COMPILER_CORE_SRC_COMPILER_IR_GRAPH_QUANTIZATION_QUANTIZE_OP_HPP
#define BACKEND_GRAPH_COMPILER_CORE_SRC_COMPILER_IR_GRAPH_QUANTIZATION_QUANTIZE_OP_HPP

#include <memory>
#include <vector>
#include "quantize_info.hpp"
#include <compiler/ir/graph/graph.hpp>
#include <compiler/ir/graph/graph_op.hpp>
#include <compiler/ir/graph/traits.hpp>
namespace sc {
namespace quantize {
/**
 * A temporary quantize and requantize op before calculation for conversion from
 * frontend. Support bf16 and int8/uint8 quantization.
 * */
class quantize_op_t : public graph_op_t, public op_traits::auto_copyable_t {
public:
    quantize_op_t(const std::vector<graph_tensor_ptr> &ins,
            const std::vector<graph_tensor_ptr> &outs, const any_map_t &attrs);
    quantize_op_t(const std::vector<graph_tensor_ptr> &ins,
            const any_map_t &attrs = any_map_t());
    std::shared_ptr<sc_graph_t> get_graph_impl() override;
    void query_format(context_ptr ctx,
            std::vector<std::vector<format_stride_pair>> &supported_ins,
            std::vector<std::vector<format_stride_pair>> &supported_outs)
            override;
};

// dequantize op
class dequantize_op_t : public graph_op_t, public op_traits::auto_copyable_t {
public:
    dequantize_op_t(const std::vector<graph_tensor_ptr> &ins,
            const std::vector<graph_tensor_ptr> &outs, const any_map_t &attrs);
    dequantize_op_t(const std::vector<graph_tensor_ptr> &ins,
            const any_map_t &attrs = any_map_t());

    std::shared_ptr<sc_graph_t> get_graph_impl() override;
    void query_format(context_ptr ctx,
            std::vector<std::vector<format_stride_pair>> &supported_ins,
            std::vector<std::vector<format_stride_pair>> &supported_outs)
            override;
};
} // namespace quantize
} // namespace sc
#endif