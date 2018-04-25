/*******************************************************************************
 * Copyright 2017-2018 Intel Corporation
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
#include <ostream>

#include "tensorflow/core/framework/op_kernel.h"
#include "tensorflow/core/graph/graph.h"

namespace tf = tensorflow;

void DumpGraph(std::string label, tf::Graph* graph);
void SummarizeOp(tf::OpKernelConstruction* ctx, std::ostream& out);
std::string GraphToDot(tf::Graph* graph, const std::string& title,
                       bool annotate_device);