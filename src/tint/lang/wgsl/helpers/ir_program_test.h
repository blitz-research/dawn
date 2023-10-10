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

#ifndef SRC_TINT_LANG_WGSL_HELPERS_IR_PROGRAM_TEST_H_
#define SRC_TINT_LANG_WGSL_HELPERS_IR_PROGRAM_TEST_H_

#include <memory>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::wgsl::helpers {

/// Helper class for testing IR with an input WGSL program.
template <typename BASE>
class IRProgramTestBase : public BASE, public ProgramBuilder {
  public:
    IRProgramTestBase() = default;
    ~IRProgramTestBase() override = default;

    /// Builds a core-dialect module from this ProgramBuilder.
    /// @returns the generated core-dialect module
    tint::Result<core::ir::Module> Build() {
        Program program{resolver::Resolve(*this)};
        if (!program.IsValid()) {
            return Failure{program.Diagnostics()};
        }

        auto result = wgsl::reader::ProgramToIR(program);
        if (!result) {
            return result.Failure();
        }

        // WGSL-dialect -> core-dialect
        if (auto lower = wgsl::reader::Lower(result.Get()); !lower) {
            return lower.Failure();
        }

        if (auto validate = core::ir::Validate(result.Get()); !validate) {
            return validate.Failure();
        }
        return result;
    }

    /// Build the module from the given WGSL.
    /// @param wgsl the WGSL to convert to IR
    /// @returns the generated module
    Result<core::ir::Module> Build(std::string wgsl) {
#if TINT_BUILD_WGSL_READER
        Source::File file("test.wgsl", std::move(wgsl));
        auto result = wgsl::reader::WgslToIR(&file);
        if (result) {
            auto validated = core::ir::Validate(result.Get());
            if (!validated) {
                return validated.Failure();
            }
        }
        return result;
#else
        (void)wgsl;
        return Failure{"error: Tint not built with the WGSL reader"};
#endif
    }
};

using IRProgramTest = IRProgramTestBase<testing::Test>;

template <typename T>
using IRProgramTestParam = IRProgramTestBase<testing::TestWithParam<T>>;

}  // namespace tint::wgsl::helpers

#endif  // SRC_TINT_LANG_WGSL_HELPERS_IR_PROGRAM_TEST_H_
