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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_LOWER_ATOMICS_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_LOWER_ATOMICS_H_

#include <string>

#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/builtin_fn.h"

namespace tint::spirv::reader {

/// Atomics is a transform that replaces calls to stub functions created by the SPIR-V reader
/// with calls to the WGSL atomic builtin. It also makes sure to replace variable declarations that
/// are the target of the atomic operations with an atomic declaration of the same type. For
/// structs, it creates a copy of the original struct with atomic members.
class Atomics final : public Castable<Atomics, ast::transform::Transform> {
  public:
    /// Constructor
    Atomics();
    /// Destructor
    ~Atomics() override;

    /// Stub is an attribute applied to stub SPIR-V reader generated functions that need to be
    /// translated to an atomic builtin.
    class Stub final : public Castable<Stub, ast::InternalAttribute> {
      public:
        /// @param pid the identifier of the program that owns this node
        /// @param nid the unique node identifier
        /// @param builtin the atomic builtin this stub represents
        Stub(GenerationID pid, ast::NodeID nid, wgsl::BuiltinFn builtin);
        /// Destructor
        ~Stub() override;

        /// @return a short description of the internal attribute which will be
        /// displayed as `@internal(<name>)`
        std::string InternalName() const override;

        /// Performs a deep clone of this object using the program::CloneContext `ctx`.
        /// @param ctx the clone context
        /// @return the newly cloned object
        const Stub* Clone(ast::CloneContext& ctx) const override;

        /// The type of the intrinsic
        const wgsl::BuiltinFn builtin;
    };

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::spirv::reader

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_LOWER_ATOMICS_H_
