/*******************************************************************************
 * Copyright 2019 Intel Corporation
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

#include "tensorflow/core/graph/graph.h"
#include "tensorflow/core/graph/node_builder.h"
#include "tensorflow/core/graph/types.h"

#include "ngraph_bridge/ngraph_mark_for_clustering.h"
#include "ngraph_bridge/ngraph_utils.h"

using namespace std;

namespace tensorflow {

namespace ngraph_bridge {

Status ReplaceOptimizer(Graph* graph, Node* node, Node** replacement,
                        const string replacement_node_name,
                        const string replacement_node_type,
                        const bool just_looking, const bool is_tf_just_looking,
                        const bool outputs_ng_supported, const int graph_id,
                        const bool is_backend_set) {
  NGRAPH_VLOG(1) << "Start replacing " << node->type_string() << " "
                 << node->name();

  std::vector<NodeBuilder::NodeOut> op_inputs;
  std::vector<const Edge*> input_edges;
  TF_RETURN_IF_ERROR(node->input_edges(&input_edges));

  NGRAPH_VLOG(1) << "No of input edges to Optimizer " << node->type_string()
                 << ": " << input_edges.size();

  int num_inputs = node->num_inputs();
  for (int i = 0; i < num_inputs; i++) {
    op_inputs.push_back(NodeBuilder::NodeOut(input_edges[i]->src(),
                                             input_edges[i]->src_output()));
  }

  NodeBuilder nb = NodeBuilder(replacement_node_name, replacement_node_type)
                       .Attr("just_looking", just_looking)
                       .Attr("is_tf_just_looking", is_tf_just_looking)
                       .Attr("copy_to_tf", !outputs_ng_supported)
                       .Attr("ngraph_graph_id", graph_id)
                       .Device(node->assigned_device_name());
  // Threre can be varying no of attributes for  different Op ex. Momentum or
  // SGD or Adam
  // for loop below generalizes this
  for (auto it = node->attrs().begin(); it != node->attrs().end(); it++) {
    nb.Attr(it->first, it->second);
  }
  // Threre can be varying no of inputs for different Op ex. Momentum or SGD or
  // Adam
  // for loop below generalizes this
  for (auto const i : op_inputs) {
    // Adding the all inputs
    nb.Input(i);
  }

  Status status = nb.Finalize(graph, replacement);
  TF_RETURN_IF_ERROR(status);
  (*replacement)->set_assigned_device_name(node->assigned_device_name());

  if (is_backend_set) {
    std::string backend_name;
    TF_RETURN_IF_ERROR(
        GetNodeAttr(node->attrs(), "_ngraph_backend", &backend_name));
    SetNodeBackend(*replacement, backend_name);
  }
  return Status::OK();
}  // end of ReplaceOptimizer

Status ReplaceAssign(Graph* graph, Node* node, Node** replacement,
                     const string replacement_node_name,
                     const string replacement_node_type,
                     const bool just_looking, const bool is_tf_just_looking,
                     const bool outputs_ng_supported, const int graph_id,
                     const bool is_backend_set) {
  NGRAPH_VLOG(1) << "Replacing  " << node->name();

  DataType dtype;
  TF_RETURN_IF_ERROR(GetNodeAttr(node->attrs(), "T", &dtype));

  NodeBuilder::NodeOut input_ref;
  NodeBuilder::NodeOut input_val;

  for (auto edge : node->in_edges()) {
    if (edge == NULL) {
      NGRAPH_VLOG(1) << "Replacing " << replacement_node_type
                     << ", found null edge: ";
      continue;
    }
    if (edge->dst()->IsOp() && !edge->IsControlEdge() &&
        IsRefType(edge->dst()->input_type(edge->dst_input()))) {
      input_ref = NodeBuilder::NodeOut(edge->src(), edge->src_output());
    } else {
      input_val = NodeBuilder::NodeOut(edge->src(), edge->src_output());
    }
  }

  TF_RETURN_IF_ERROR(NodeBuilder(replacement_node_name, replacement_node_type)
                         .Attr("validate_shape", true)
                         .Attr("use_locking", true)
                         .Attr("T", dtype)
                         .Attr("just_looking", just_looking)
                         .Attr("is_tf_just_looking", is_tf_just_looking)
                         .Attr("copy_to_tf", !outputs_ng_supported)
                         .Attr("ngraph_graph_id", graph_id)
                         .Input(input_ref)
                         .Input(input_val)
                         .Device(node->assigned_device_name())
                         .Finalize(graph, replacement));

  (*replacement)->set_assigned_device_name(node->assigned_device_name());

  if (is_backend_set) {
    std::string backend_name;
    TF_RETURN_IF_ERROR(
        GetNodeAttr(node->attrs(), "_ngraph_backend", &backend_name));
    SetNodeBackend(*replacement, backend_name);
  }

  NGRAPH_VLOG(4) << "Replacing Node " << node->DebugString() << " with "
                 << (*replacement)->DebugString();
  return Status::OK();
}

Status ReplaceVariable(Graph* graph, Node* node, Node** replacement,
                       const string replacement_node_name,
                       const string replacement_node_type,
                       const bool just_looking, const bool is_tf_just_looking,
                       const bool outputs_ng_supported, const int graph_id,
                       const bool is_backend_set) {
  NGRAPH_VLOG(1) << "Replacing NGraphVariable " << node->name();

  TensorShape shape;
  DataType dtype;
  TF_RETURN_IF_ERROR(GetNodeAttr(node->attrs(), "shape", &shape));
  TF_RETURN_IF_ERROR(GetNodeAttr(node->attrs(), "dtype", &dtype));

  std::string container;
  std::string shared_name;

  if (GetNodeAttr(node->attrs(), "container", &container) != Status::OK()) {
    container = "";
  }
  if (GetNodeAttr(node->attrs(), "shared_name", &shared_name) != Status::OK()) {
    shared_name = "";
  }

  TF_RETURN_IF_ERROR(
      NodeBuilder(replacement_node_name, replacement_node_type)
          .Attr("shape", shape)
          .Attr("dtype", dtype)
          .Attr("container", container)
          .Attr("shared_name",
                (shared_name.empty() ? node->name() : shared_name))
          .Attr("just_looking", just_looking)
          .Attr("is_tf_just_looking", is_tf_just_looking)
          .Attr("copy_to_tf", !outputs_ng_supported)
          .Attr("ngraph_graph_id", graph_id)
          .Device(node->assigned_device_name())
          .Finalize(graph, &(*replacement)));

  (*replacement)->set_assigned_device_name(node->assigned_device_name());

  if (is_backend_set) {
    std::string backend_name;
    TF_RETURN_IF_ERROR(
        GetNodeAttr(node->attrs(), "_ngraph_backend", &backend_name));
    SetNodeBackend(*replacement, backend_name);
  }
  NGRAPH_VLOG(4) << "Replacing Node " << node->DebugString() << " with "
                 << (*replacement)->DebugString();

  return Status::OK();
}

// Though edges will be removed when we remove the node
// we specifically remove the edges to be sure
Status ReplaceInputControlEdges(Graph* graph, Node* node, Node* replacement) {
  std::vector<const Edge*> edges_to_remove;
  for (auto edge : node->in_edges()) {
    NGRAPH_VLOG(4) << "Replacing: " << edge->DebugString();
    if (!edge->IsControlEdge()) continue;
    graph->AddEdge(edge->src(), edge->src_output(), replacement,
                   edge->dst_input());
    edges_to_remove.push_back(edge);
  }
  for (auto edge : edges_to_remove) {
    graph->RemoveEdge(edge);
  }
  return Status::OK();
}

// Though edges will be removed when we remove the node
// we specifically remove the edges to be sure
Status ReplaceOutputEdges(Graph* graph, Node* node, Node* replacement) {
  std::vector<const Edge*> edges;
  std::vector<const Edge*> edges_to_remove;
  for (auto edge : node->out_edges()) {
    edges.push_back(edge);
  }

  for (auto edge : edges) {
    NGRAPH_VLOG(4) << "Replacing: " << edge->DebugString();
    graph->AddEdge(replacement, edge->src_output(), edge->dst(),
                   edge->dst_input());
    edges_to_remove.push_back(edge);
  }
  for (auto edge : edges_to_remove) {
    graph->RemoveEdge(edge);
  }
  return Status::OK();
}

bool IsValidateShape(Node* node) {
  bool validate_shape;
  GetNodeAttr(node->attrs(), "validate_shape", &validate_shape);
  return validate_shape;
}

Status StoreRefTypeOutputs(Node* node, std::set<Node*>* ref_list) {
  for (auto edge : node->out_edges()) {
    // no need to go over control edges
    if (!(edge->IsControlEdge()) && ref_list->size()) {
      Node* dst = edge->dst();
      if (IsRefType(dst->input_type(edge->dst_input()))) {
        NGRAPH_VLOG(4) << "Found a ref type output " << dst->name();

        // Check if the node is Assign node and it's attribute validate_shape
        if (dst->type_string() == "Assign" && !IsValidateShape(dst)) {
          NGRAPH_VLOG(4) << "The attribute validate_shape for Assign is false ";
          // If the dst node is Assign and attr: validate_shape is false then
          // we do not want to capture the Assign or any of the ref nodes
          // leading from it or the variable node or ref nodes that lead to it.
          // So clear the ref list and return.
          ref_list->clear();
          break;
        } else {
          // In all other cases:
          // 1. Assign, validate_shape = true
          // 2. AssignAdd
          // 3. AssignSub
          // 4. ApplyGradientDescent
          // 5. other
          // which are all ref types, add to the ref list if not already added
          if (ref_list->find(dst) == ref_list->end()) {
            NGRAPH_VLOG(4) << "Adding " << dst->name() << " to the ref list";
            ref_list->insert(dst);
          } else {
            NGRAPH_VLOG(4) << "Possible cycle in the graph.";
          }
        }

        // Recursively go over the outputs of each ref type output.
        StoreRefTypeOutputs(dst, ref_list);
      }
    }
  }
  return Status::OK();
}

}  // namespace ngraph_bridge

}  // namespace tensorflow
