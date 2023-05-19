// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_IR_LOAD_H_
#define SRC_TINT_IR_LOAD_H_

#include "src/tint/ir/instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A load instruction in the IR.
class Load : public utils::Castable<Load, Instruction> {
  public:
    /// Constructor
    /// @param type the result type
    /// @param from the value being loaded from
    Load(const type::Type* type, Value* from);
    Load(const Load& inst) = delete;
    Load(Load&& inst) = delete;
    ~Load() override;

    Load& operator=(const Load& inst) = delete;
    Load& operator=(Load&& inst) = delete;

    /// @returns the type of the value
    const type::Type* Type() const override { return result_type_; }

    /// @returns the avlue being loaded from
    Value* From() const { return from_; }

  private:
    const type::Type* result_type_;
    Value* from_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_LOAD_H_
