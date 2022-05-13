/*******************************************************************************
* Copyright 2021-2022 Intel Corporation
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

#include "backend/dnnl/internal_ops.hpp"
#include "backend/dnnl/patterns/fusions.hpp"

#include "utils/pm/pbuilder.hpp"

namespace dnnl {
namespace graph {
namespace impl {
namespace dnnl_impl {
namespace pass {

namespace pm = impl::utils::pm;
using in_edges_t = pm::in_edges_t;
using pb_graph_t = pm::pb_graph_t;
using FCreateV2FusedOp = impl::pass::FCreateV2FusedOp;
using FCreateV2Pattern = impl::pass::FCreateV2Pattern;

namespace {
bool check_attributes(op_t *graph_op) {
    if (graph_op->has_attr("axes")
            && graph_op->get_attr<std::vector<int64_t>>("axes").empty())
        return false;
    return true;
}
} // namespace

/*!
 * \brief This provides reduction fusion.
 *        The process includes follow steps:
 *          1. look for fusion pattern on the graph
 *          2. If found, verify if this transformation is safe / correct
 *          3. replace the pattern with a fused op, update the graph
 */
DNNL_BACKEND_REGISTER_PASSES_DEF_BEGIN(reduction_fusion)

DNNL_BACKEND_REGISTER_TRANSFORMATION_PASS(dnnl, reduction_post_ops_fusion)
        .set_priority(8.3f)
        .set_attr<FCreateV2Pattern>("FCreateV2Pattern",
                [](const std::shared_ptr<pb_graph_t> &pgraph) -> void {
                    pm::pb_op *reduction = pgraph->append_alternation(
                            {impl::op_kind::ReduceL1, impl::op_kind::ReduceL2,
                                    impl::op_kind::ReduceMax,
                                    impl::op_kind::ReduceMean,
                                    impl::op_kind::ReduceMin,
                                    impl::op_kind::ReduceProd,
                                    impl::op_kind::ReduceSum});
                    reduction->append_decision_function(check_attributes);

                    // special post op handle: swish is composed
                    // by sigmoid and multiply
                    auto swish_graph
                            = std::make_shared<pb_graph_t>("swish_graph");
                    auto psigmoid = swish_graph->append_op(
                            impl::op_kind::Sigmoid, "psigmoid");
                    auto pmultiply
                            = swish_graph->append_op(impl::op_kind::Multiply,
                                    {in_edge(0, psigmoid, 0)}, "pmultiply");
                    swish_graph->create_input_port(0, psigmoid, 0);
                    swish_graph->create_input_port(0, pmultiply, 1);
                    swish_graph->create_output_port(0, pmultiply, 0);

                    auto other_postop_graph = std::make_shared<pb_graph_t>(
                            "pother_postop_graph");
                    pm::pb_op *pop = other_postop_graph->append_alternation(
                            {impl::op_kind::Abs, impl::op_kind::Clamp,
                                    impl::op_kind::Elu, impl::op_kind::Exp,
                                    impl::op_kind::GELU,
                                    impl::op_kind::HardTanh,
                                    impl::op_kind::HardSwish,
                                    impl::op_kind::Log, impl::op_kind::Sigmoid,
                                    impl::op_kind::SoftPlus, impl::op_kind::Pow,
                                    impl::op_kind::ReLU, impl::op_kind::Round,
                                    impl::op_kind::Sqrt, impl::op_kind::Square,
                                    impl::op_kind::Tanh, impl::op_kind::Add,
                                    impl::op_kind::Multiply,
                                    impl::op_kind::Maximum,
                                    impl::op_kind::Minimum,
                                    impl::op_kind::Divide,
                                    impl::op_kind::Subtract},
                            "pother_post_op");
                    other_postop_graph->create_input_port(0, pop, 0);
                    other_postop_graph->create_output_port(0, pop, 0);

                    pgraph->append_alternation(
                            {swish_graph, other_postop_graph},
                            in_edges_t {in_edge(0, reduction, 0)}, "ppost_op");
                })
        .set_attr<FCreateV2FusedOp>(
                "FCreateV2FusedOp", []() -> std::shared_ptr<op_t> {
                    std::shared_ptr<op_t> fused_op = std::make_shared<op_t>(
                            op_kind::reduction_post_ops_fusion);
                    fused_op->set_attr<std::string>("backend", "dnnl");
                    return fused_op;
                });

DNNL_BACKEND_REGISTER_PASSES_DEF_END

} // namespace pass
} // namespace dnnl_impl
} // namespace impl
} // namespace graph
} // namespace dnnl