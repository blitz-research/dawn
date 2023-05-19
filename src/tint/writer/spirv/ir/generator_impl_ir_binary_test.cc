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

#include "src/tint/writer/spirv/ir/test_helper_ir.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::spirv {
namespace {

TEST_F(SpvGeneratorImplTest, Binary_Add_I32) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    func->StartTarget()->SetInstructions(
        utils::Vector{b.Add(mod.types.Get<type::I32>(), b.Constant(1_i), b.Constant(2_i))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpIAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Add_U32) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    func->StartTarget()->SetInstructions(
        utils::Vector{b.Add(mod.types.Get<type::U32>(), b.Constant(1_u), b.Constant(2_u))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpIAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Add_F32) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    func->StartTarget()->SetInstructions(
        utils::Vector{b.Add(mod.types.Get<type::F32>(), b.Constant(1_f), b.Constant(2_f))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpFAdd %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_I32) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.types.Get<type::I32>(), b.Constant(1_i), b.Constant(2_i))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_U32) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.types.Get<type::U32>(), b.Constant(1_u), b.Constant(2_u))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 0
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_F32) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.types.Get<type::F32>(), b.Constant(1_f), b.Constant(2_f))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeFloat 32
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpFSub %6 %7 %8
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_Vec2i) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    auto* lhs = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(mod.types.Get<type::I32>(), 2u),
        utils::Vector{b.Constant(42_i)->Value(), b.Constant(-1_i)->Value()}, false, false);
    auto* rhs = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(mod.types.Get<type::I32>(), 2u),
        utils::Vector{b.Constant(0_i)->Value(), b.Constant(-43_i)->Value()}, false, false);
    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.types.Get<type::Vector>(mod.types.Get<type::I32>(), 2u),
                                 b.Constant(lhs), b.Constant(rhs))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeInt 32 1
%6 = OpTypeVector %7 2
%9 = OpConstant %7 42
%10 = OpConstant %7 -1
%8 = OpConstantComposite %6 %9 %10
%12 = OpConstant %7 0
%13 = OpConstant %7 -43
%11 = OpConstantComposite %6 %12 %13
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %8 %11
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Sub_Vec4f) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    auto* lhs = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(mod.types.Get<type::F32>(), 4u),
        utils::Vector{b.Constant(42_f)->Value(), b.Constant(-1_f)->Value(),
                      b.Constant(0_f)->Value(), b.Constant(1.25_f)->Value()},
        false, false);
    auto* rhs = mod.constants_arena.Create<constant::Composite>(
        mod.types.Get<type::Vector>(mod.types.Get<type::F32>(), 4u),
        utils::Vector{b.Constant(0_f)->Value(), b.Constant(1.25_f)->Value(),
                      b.Constant(-42_f)->Value(), b.Constant(1_f)->Value()},
        false, false);
    func->StartTarget()->SetInstructions(
        utils::Vector{b.Subtract(mod.types.Get<type::Vector>(mod.types.Get<type::F32>(), 4u),
                                 b.Constant(lhs), b.Constant(rhs))});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%7 = OpTypeFloat 32
%6 = OpTypeVector %7 4
%9 = OpConstant %7 42
%10 = OpConstant %7 -1
%11 = OpConstant %7 0
%12 = OpConstant %7 1.25
%8 = OpConstantComposite %6 %9 %10 %11 %12
%14 = OpConstant %7 -42
%15 = OpConstant %7 1
%13 = OpConstantComposite %6 %11 %12 %14 %15
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpFSub %6 %8 %13
OpReturn
OpFunctionEnd
)");
}

TEST_F(SpvGeneratorImplTest, Binary_Chain) {
    auto* func = b.CreateFunction(mod.symbols.Register("foo"), mod.types.Get<type::Void>());
    func->StartTarget()->BranchTo(func->EndTarget());

    auto* a = b.Subtract(mod.types.Get<type::I32>(), b.Constant(1_i), b.Constant(2_i));
    func->StartTarget()->SetInstructions(utils::Vector{a, b.Add(mod.types.Get<type::I32>(), a, a)});

    generator_.EmitFunction(func);
    EXPECT_EQ(DumpModule(generator_.Module()), R"(OpName %1 "foo"
%2 = OpTypeVoid
%3 = OpTypeFunction %2
%6 = OpTypeInt 32 1
%7 = OpConstant %6 1
%8 = OpConstant %6 2
%1 = OpFunction %2 None %3
%4 = OpLabel
%5 = OpISub %6 %7 %8
%9 = OpIAdd %6 %5 %5
OpReturn
OpFunctionEnd
)");
}

}  // namespace
}  // namespace tint::writer::spirv
