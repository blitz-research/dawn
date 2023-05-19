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

#ifndef SRC_TINT_IR_VAR_H_
#define SRC_TINT_IR_VAR_H_

#include "src/tint/builtin/access.h"
#include "src/tint/builtin/address_space.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// An instruction in the IR.
class Var : public utils::Castable<Var, Instruction> {
  public:
    /// Constructor
    /// @param type the type of the var
    explicit Var(const type::Type* type);
    Var(const Var& inst) = delete;
    Var(Var&& inst) = delete;
    ~Var() override;

    Var& operator=(const Var& inst) = delete;
    Var& operator=(Var&& inst) = delete;

    /// @returns the type of the var
    const type::Type* Type() const override { return type_; }

    /// Sets the var initializer
    /// @param initializer the initializer
    void SetInitializer(Value* initializer);
    /// @returns the initializer
    const Value* Initializer() const { return initializer_; }

  private:
    const type::Type* type_;
    Value* initializer_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_VAR_H_
