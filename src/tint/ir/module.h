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

#ifndef SRC_TINT_IR_MODULE_H_
#define SRC_TINT_IR_MODULE_H_

#include <string>

#include "src/tint/constant/value.h"
#include "src/tint/ir/constant.h"
#include "src/tint/ir/function.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/ir/value.h"
#include "src/tint/program_id.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/manager.h"
#include "src/tint/utils/block_allocator.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/vector.h"

namespace tint::ir {

/// Main module class for the IR.
class Module {
    /// Program Id required to create other components
    ProgramID prog_id_;

    /// Map of value to pre-declared identifier
    utils::Hashmap<const Value*, Symbol, 32> value_to_id_;

    /// Map of pre-declared identifier to value
    utils::Hashmap<Symbol, const Value*, 32> id_to_value_;

  public:
    /// Constructor
    Module();
    /// Move constructor
    /// @param o the module to move from
    Module(Module&& o);
    /// Destructor
    ~Module();

    /// Move assign
    /// @param o the module to assign from
    /// @returns a reference to this module
    Module& operator=(Module&& o);

    /// @param value the value
    /// @return the name of the given value, or an invalid symbol if the value is not named.
    Symbol NameOf(const Value* value) const;

    /// @param value the value to name.
    /// @param name the desired name of the value. May be suffixed on collision.
    /// @return the unique symbol of the given value.
    Symbol SetName(const Value* value, std::string_view name);

    /// The flow node allocator
    utils::BlockAllocator<FlowNode> flow_nodes;
    /// The constant allocator
    utils::BlockAllocator<constant::Value> constants_arena;
    /// The value allocator
    utils::BlockAllocator<Value> values;

    /// List of functions in the program
    utils::Vector<Function*, 8> functions;

    /// The block containing module level declarations, if any exist.
    Block* root_block = nullptr;

    /// The type manager for the module
    type::Manager types;

    /// The symbol table for the module
    SymbolTable symbols{prog_id_};

    /// ConstantHasher provides a hash function for a constant::Value pointer, hashing the value
    /// instead of the pointer itself.
    struct ConstantHasher {
        /// @param c the constant pointer to create a hash for
        /// @return the hash value
        inline std::size_t operator()(const constant::Value* c) const { return c->Hash(); }
    };

    /// ConstantEquals provides an equality function for two constant::Value pointers, comparing
    /// their values instead of the pointers.
    struct ConstantEquals {
        /// @param a the first constant pointer to compare
        /// @param b the second constant pointer to compare
        /// @return the hash value
        inline bool operator()(const constant::Value* a, const constant::Value* b) const {
            return a->Equal(b);
        }
    };

    /// The map of constant::Value to their ir::Constant.
    utils::Hashmap<const constant::Value*, ir::Constant*, 16, ConstantHasher, ConstantEquals>
        constants;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_MODULE_H_
