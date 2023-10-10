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

#include <utility>

#include "src/tint/lang/core/ir/transform/helper_test.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/wgsl/writer/raise/raise.h"

namespace tint::wgsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using WgslWriter_RaiseTest = core::ir::transform::TransformTest;

TEST_F(WgslWriter_RaiseTest, BuiltinConversion) {
    auto* f = b.Function("f", ty.void_());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.i32(), core::BuiltinFn::kMax, i32(1), i32(2));
        b.Return(f);
    });

    auto* src = R"(
%f = func():void -> %b1 {
  %b1 = block {
    %2:i32 = max 1i, 2i
    ret
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%f = func():void -> %b1 {
  %b1 = block {
    %2:i32 = wgsl.max 1i, 2i
    ret
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_RaiseTest, WorkgroupBarrier) {
    auto* W = b.Var<workgroup, i32, read_write>("W");
    b.ir.root_block->Append(W);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        auto* load = b.Load(W);
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(f, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:void = workgroupBarrier
    %4:i32 = load %W
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:i32 = wgsl.workgroupUniformLoad %W
    ret %3
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

TEST_F(WgslWriter_RaiseTest, WorkgroupBarrier_NoMatch) {
    auto* W = b.Var<workgroup, i32, read_write>("W");
    b.ir.root_block->Append(W);
    auto* f = b.Function("f", ty.i32());
    b.Append(f->Block(), [&] {  //
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Store(W, 42_i);  // Prevents pattern match
        auto* load = b.Load(W);
        b.Call(ty.void_(), core::BuiltinFn::kWorkgroupBarrier);
        b.Return(f, load);
    });

    auto* src = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:void = workgroupBarrier
    store %W, 42i
    %4:i32 = load %W
    %5:void = workgroupBarrier
    ret %4
  }
}
)";
    EXPECT_EQ(src, str());

    auto* expect = R"(
%b1 = block {  # root
  %W:ptr<workgroup, i32, read_write> = var
}

%f = func():i32 -> %b2 {
  %b2 = block {
    %3:void = wgsl.workgroupBarrier
    store %W, 42i
    %4:i32 = load %W
    %5:void = wgsl.workgroupBarrier
    ret %4
  }
}
)";

    Run(Raise);

    EXPECT_EQ(expect, str());
}

}  // namespace
}  // namespace tint::wgsl::writer::raise
