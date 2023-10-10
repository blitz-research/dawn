// Copyright 2020 The Tint Authors.
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

#include "src/tint/lang/spirv/writer/writer.h"

#include <memory>
#include <utility>

#include "src/tint/lang/spirv/writer/ast_printer/ast_printer.h"
#include "src/tint/lang/spirv/writer/common/option_builder.h"
#include "src/tint/lang/spirv/writer/printer/printer.h"
#include "src/tint/lang/spirv/writer/raise/raise.h"
#include "src/tint/lang/wgsl/reader/lower/lower.h"
#include "src/tint/lang/wgsl/reader/program_to_ir/program_to_ir.h"

// Included by 'ast_printer.h', included again here for './tools/run gen' track the dependency.
#include "spirv/unified1/spirv.h"

namespace tint::spirv::writer {

Output::Output() = default;
Output::~Output() = default;
Output::Output(const Output&) = default;

Result<Output> Generate(const Program& program, const Options& options) {
    if (!program.IsValid()) {
        return Failure{program.Diagnostics()};
    }

    bool zero_initialize_workgroup_memory =
        !options.disable_workgroup_init && options.use_zero_initialize_workgroup_memory_extension;

    {
        diag::List validation_diagnostics;
        if (!ValidateBindingOptions(options, validation_diagnostics)) {
            return Failure{validation_diagnostics};
        }
    }

    Output output;

    if (options.use_tint_ir) {
        // Convert the AST program to an IR module.
        auto converted = wgsl::reader::ProgramToIR(program);
        if (!converted) {
            return converted.Failure();
        }

        auto ir = converted.Move();

        // Lower from WGSL-dialect to core-dialect
        if (auto res = wgsl::reader::Lower(ir); !res) {
            return res.Failure();
        }

        // Raise from core-dialect to SPIR-V-dialect.
        if (auto res = raise::Raise(ir, options); !res) {
            return std::move(res.Failure());
        }

        // Generate the SPIR-V code.
        auto impl = std::make_unique<Printer>(ir, zero_initialize_workgroup_memory);
        auto spirv = impl->Generate();
        if (!spirv) {
            return std::move(spirv.Failure());
        }
        output.spirv = std::move(spirv.Get());
    } else {
        // Sanitize the program.
        auto sanitized_result = Sanitize(program, options);
        if (!sanitized_result.program.IsValid()) {
            return Failure{sanitized_result.program.Diagnostics()};
        }

        // Generate the SPIR-V code.
        auto impl = std::make_unique<ASTPrinter>(
            sanitized_result.program, zero_initialize_workgroup_memory,
            options.experimental_require_subgroup_uniform_control_flow);
        if (!impl->Generate()) {
            return Failure{impl->Diagnostics()};
        }
        output.spirv = std::move(impl->Result());
    }

    return output;
}

}  // namespace tint::spirv::writer
