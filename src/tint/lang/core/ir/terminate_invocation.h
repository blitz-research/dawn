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

#ifndef SRC_TINT_LANG_CORE_IR_TERMINATE_INVOCATION_H_
#define SRC_TINT_LANG_CORE_IR_TERMINATE_INVOCATION_H_

#include <string>

#include "src/tint/lang/core/ir/terminator.h"

namespace tint::core::ir {

/// An terminate invocation instruction in the IR.
class TerminateInvocation final : public Castable<TerminateInvocation, Terminator> {
  public:
    ~TerminateInvocation() override;

    /// @copydoc Instruction::Clone()
    TerminateInvocation* Clone(CloneContext& ctx) override;

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "terminate_invocation"; }
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_TERMINATE_INVOCATION_H_
