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

#include "src/tint/ir/test_helper.h"

#include "gmock/gmock.h"
#include "src/tint/ast/case_selector.h"
#include "src/tint/ast/int_literal_expression.h"
#include "src/tint/constant/scalar.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT

using IR_BuilderImplTest = TestHelper;

TEST_F(IR_BuilderImplTest, EmitFunction_Vertex) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(0_f, 0_f, 0_f, 0_f))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():vec4<f32> [@vertex ra: @position]
  %fn1 = block
  ret vec4<f32> 0.0f
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_Fragment) {
    Func("test", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kFragment)});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():void [@fragment]
  %fn1 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_Compute) {
    Func("test", utils::Empty, ty.void_(), utils::Empty,
         utils::Vector{Stage(ast::PipelineStage::kCompute), WorkgroupSize(8_i, 4_i, 2_i)});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():void [@compute @workgroup_size(8, 4, 2)]
  %fn1 = block
  ret
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_Return) {
    Func("test", utils::Empty, ty.vec3<f32>(), utils::Vector{Return(vec3<f32>(0_f, 0_f, 0_f))},
         utils::Empty);

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():vec3<f32>
  %fn1 = block
  ret vec3<f32> 0.0f
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_ReturnPosition) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(1_f, 2_f, 3_f, 4_f))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition)});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():vec4<f32> [@vertex ra: @position]
  %fn1 = block
  ret vec4<f32> 1.0f, 2.0f, 3.0f, 4.0f
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_ReturnPositionInvariant) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(1_f, 2_f, 3_f, 4_f))},
         utils::Vector{Stage(ast::PipelineStage::kVertex)},
         utils::Vector{Builtin(builtin::BuiltinValue::kPosition), Invariant()});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():vec4<f32> [@vertex ra: @position @invariant]
  %fn1 = block
  ret vec4<f32> 1.0f, 2.0f, 3.0f, 4.0f
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_ReturnLocation) {
    Func("test", utils::Empty, ty.vec4<f32>(), utils::Vector{Return(vec4<f32>(1_f, 2_f, 3_f, 4_f))},
         utils::Vector{Stage(ast::PipelineStage::kFragment)}, utils::Vector{Location(1_i)});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():vec4<f32> [@fragment ra: @location(1)]
  %fn1 = block
  ret vec4<f32> 1.0f, 2.0f, 3.0f, 4.0f
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_ReturnFragDepth) {
    Func("test", utils::Empty, ty.f32(), utils::Vector{Return(1_f)},
         utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Builtin(builtin::BuiltinValue::kFragDepth)});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():f32 [@fragment ra: @frag_depth]
  %fn1 = block
  ret 1.0f
func_end

)");
}

TEST_F(IR_BuilderImplTest, EmitFunction_ReturnSampleMask) {
    Func("test", utils::Empty, ty.u32(), utils::Vector{Return(1_u)},
         utils::Vector{Stage(ast::PipelineStage::kFragment)},
         utils::Vector{Builtin(builtin::BuiltinValue::kSampleMask)});

    auto r = Build();
    ASSERT_TRUE(r) << Error();
    auto m = r.Move();

    EXPECT_EQ(Disassemble(m), R"(%fn0 = func test():u32 [@fragment ra: @sample_mask]
  %fn1 = block
  ret 1u
func_end

)");
}

}  // namespace
}  // namespace tint::ir
