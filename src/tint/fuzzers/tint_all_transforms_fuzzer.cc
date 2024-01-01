// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/fuzzers/fuzzer_init.h"
#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/transform_builder.h"

namespace tint::fuzzers {

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kSpv);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }

#if TINT_BUILD_HLSL_WRITER
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kHLSL);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }
#endif  // TINT_BUILD_HLSL_WRITER

#if TINT_BUILD_MSL_WRITER
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kMSL);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }
#endif  // TINT_BUILD_MSL_WRITER
#if TINT_BUILD_SPV_WRITER
    {
        TransformBuilder tb(data, size);
        tb.AddTransform<ShuffleTransform>();
        tb.AddPlatformIndependentPasses();

        fuzzers::CommonFuzzer fuzzer(InputFormat::kWGSL, OutputFormat::kSpv);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());
        fuzzer.SetDumpInput(GetCliParams().dump_input);
        fuzzer.SetEnforceValidity(GetCliParams().enforce_validity);

        fuzzer.Run(data, size);
    }
#endif  // TINT_BUILD_SPV_WRITER

    return 0;
}

}  // namespace tint::fuzzers
