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

#ifndef SRC_TINT_LANG_CORE_IR_VAR_H_
#define SRC_TINT_LANG_CORE_IR_VAR_H_

#include <string>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/core/interpolation.h"
#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// Attributes that can be applied to a variable that will be used for shader IO.
struct IOAttributes {
    /// The value of a `@location` attribute.
    std::optional<uint32_t> location;
    /// The value of a `@index` attribute.
    std::optional<uint32_t> index;
    /// The value of a `@builtin` attribute.
    std::optional<core::BuiltinValue> builtin;
    /// The values of a `@interpolate` attribute.
    std::optional<core::Interpolation> interpolation;
    /// True if the variable is annotated with `@invariant`.
    bool invariant = false;
};

/// A var instruction in the IR.
class Var final : public Castable<Var, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the initializer
    static constexpr size_t kInitializerOperandOffset = 0;

    /// Constructor
    /// @param result the result value
    explicit Var(InstructionResult* result);
    ~Var() override;

    /// @copydoc Instruction::Clone()
    Var* Clone(CloneContext& ctx) override;

    /// Sets the var initializer
    /// @param initializer the initializer
    void SetInitializer(Value* initializer);
    /// @returns the initializer
    Value* Initializer() { return operands_[kInitializerOperandOffset]; }

    /// Sets the binding point
    /// @param group the group
    /// @param binding the binding
    void SetBindingPoint(uint32_t group, uint32_t binding) { binding_point_ = {group, binding}; }
    /// @returns the binding points if `Attributes` contains `kBindingPoint`
    std::optional<struct BindingPoint> BindingPoint() { return binding_point_; }

    /// Sets the IO attributes
    /// @param attrs the attributes
    void SetAttributes(const IOAttributes& attrs) { attributes_ = attrs; }
    /// @returns the IO attributes
    const IOAttributes& Attributes() { return attributes_; }

    /// Destroys this instruction along with any assignment instructions, if the var is never read.
    void DestroyIfOnlyAssigned();

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "var"; }

  private:
    std::optional<struct BindingPoint> binding_point_;
    IOAttributes attributes_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_VAR_H_
