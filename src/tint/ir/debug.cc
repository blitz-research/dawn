// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/ir/debug.h"

#include <unordered_map>
#include <unordered_set>

#include "src/tint/ir/block.h"
#include "src/tint/ir/function_terminator.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/switch.h"
#include "src/tint/switch.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

// static
std::string Debug::AsDotGraph(const Module* mod) {
    size_t node_count = 0;

    std::unordered_set<const FlowNode*> visited;
    std::unordered_set<const FlowNode*> merge_nodes;
    std::unordered_map<const FlowNode*, std::string> node_to_name;
    utils::StringStream out;

    auto name_for = [&](const FlowNode* node) -> std::string {
        if (node_to_name.count(node) > 0) {
            return node_to_name[node];
        }

        std::string name = "node_" + std::to_string(node_count);
        node_count += 1;

        node_to_name[node] = name;
        return name;
    };

    std::function<void(const FlowNode*)> Graph = [&](const FlowNode* node) {
        if (visited.count(node) > 0) {
            return;
        }
        visited.insert(node);

        tint::Switch(
            node,
            [&](const ir::Block* b) {
                if (node_to_name.count(b) == 0) {
                    out << name_for(b) << R"( [label="block"])" << std::endl;
                }
                out << name_for(b) << " -> " << name_for(b->Branch().target);

                // Dashed lines to merge blocks
                if (merge_nodes.count(b->Branch().target) != 0) {
                    out << " [style=dashed]";
                }

                out << std::endl;
                Graph(b->Branch().target);
            },
            [&](const ir::Switch* s) {
                out << name_for(s) << R"( [label="switch"])" << std::endl;
                out << name_for(s->Merge().target) << R"( [label="switch merge"])" << std::endl;
                merge_nodes.insert(s->Merge().target);

                size_t i = 0;
                for (const auto& c : s->Cases()) {
                    out << name_for(c.Start().target)
                        << R"( [label="case )" + std::to_string(i++) + R"("])" << std::endl;
                }
                out << name_for(s) << " -> {";
                for (const auto& c : s->Cases()) {
                    if (&c != &(s->Cases().Front())) {
                        out << ", ";
                    }
                    out << name_for(c.Start().target);
                }
                out << "}" << std::endl;

                for (const auto& c : s->Cases()) {
                    Graph(c.Start().target);
                }
                Graph(s->Merge().target);
            },
            [&](const ir::If* i) {
                out << name_for(i) << R"( [label="if"])" << std::endl;
                out << name_for(i->True().target) << R"( [label="true"])" << std::endl;
                out << name_for(i->False().target) << R"( [label="false"])" << std::endl;
                out << name_for(i->Merge().target) << R"( [label="if merge"])" << std::endl;
                merge_nodes.insert(i->Merge().target);

                out << name_for(i) << " -> {";
                out << name_for(i->True().target) << ", " << name_for(i->False().target);
                out << "}" << std::endl;

                // Subgraph if true/false branches so they draw on the same line
                out << "subgraph sub_" << name_for(i) << " {" << std::endl;
                out << R"(rank="same")" << std::endl;
                out << name_for(i->True().target) << std::endl;
                out << name_for(i->False().target) << std::endl;
                out << "}" << std::endl;

                Graph(i->True().target);
                Graph(i->False().target);
                Graph(i->Merge().target);
            },
            [&](const ir::Loop* l) {
                out << name_for(l) << R"( [label="loop"])" << std::endl;
                out << name_for(l->Start().target) << R"( [label="start"])" << std::endl;
                out << name_for(l->Continuing().target) << R"( [label="continuing"])" << std::endl;
                out << name_for(l->Merge().target) << R"( [label="loop merge"])" << std::endl;
                merge_nodes.insert(l->Merge().target);

                // Subgraph the continuing and merge so they get drawn on the same line
                out << "subgraph sub_" << name_for(l) << " {" << std::endl;
                out << R"(rank="same")" << std::endl;
                out << name_for(l->Continuing().target) << std::endl;
                out << name_for(l->Merge().target) << std::endl;
                out << "}" << std::endl;

                out << name_for(l) << " -> " << name_for(l->Start().target) << std::endl;

                Graph(l->Start().target);
                Graph(l->Continuing().target);
                Graph(l->Merge().target);
            },
            [&](const ir::FunctionTerminator*) {
                // Already done
            });
    };

    out << "digraph G {" << std::endl;
    for (const auto* func : mod->functions) {
        // Cluster each function to label and draw a box around it.
        out << "subgraph cluster_" << name_for(func) << " {" << std::endl;
        out << R"(label=")" << func->Name().Name() << R"(")" << std::endl;
        out << name_for(func->StartTarget()) << R"( [label="start"])" << std::endl;
        out << name_for(func->EndTarget()) << R"( [label="end"])" << std::endl;
        Graph(func->StartTarget());
        out << "}" << std::endl;
    }
    out << "}";
    return out.str();
}

}  // namespace tint::ir
