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

#ifndef SRC_TINT_LANG_SPIRV_READER_AST_LOWER_FOLD_TRIVIAL_LETS_H_
#define SRC_TINT_LANG_SPIRV_READER_AST_LOWER_FOLD_TRIVIAL_LETS_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::spirv::reader {

/// FoldTrivialLets is a transform that inlines the initializers of let declarations whose
/// initializers are just identifier expressions, or lets that are only used once. This is used to
/// clean up unnecessary let declarations created by the SPIR-V reader.
class FoldTrivialLets final : public Castable<FoldTrivialLets, ast::transform::Transform> {
  public:
    /// Constructor
    FoldTrivialLets();

    /// Destructor
    ~FoldTrivialLets() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::spirv::reader

#endif  // SRC_TINT_LANG_SPIRV_READER_AST_LOWER_FOLD_TRIVIAL_LETS_H_
