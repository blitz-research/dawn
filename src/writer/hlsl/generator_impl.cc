/// Copyright 2020 The Tint Authors.
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

#include "src/writer/hlsl/generator_impl.h"

#include <algorithm>
#include <iomanip>
#include <set>
#include <utility>
#include <vector>

#include "src/ast/call_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/internal_decoration.h"
#include "src/ast/interpolate_decoration.h"
#include "src/ast/override_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/debug.h"
#include "src/sem/array.h"
#include "src/sem/atomic_type.h"
#include "src/sem/block_statement.h"
#include "src/sem/call.h"
#include "src/sem/depth_texture_type.h"
#include "src/sem/function.h"
#include "src/sem/member_accessor_expression.h"
#include "src/sem/multisampled_texture_type.h"
#include "src/sem/sampled_texture_type.h"
#include "src/sem/statement.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/sem/variable.h"
#include "src/transform/calculate_array_length.h"
#include "src/transform/hlsl.h"
#include "src/utils/get_or_create.h"
#include "src/utils/scoped_assignment.h"
#include "src/writer/append_vector.h"
#include "src/writer/float_to_string.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

const char kTempNamePrefix[] = "tint_tmp";
const char kSpecConstantPrefix[] = "WGSL_SPEC_CONSTANT_";

bool last_is_break_or_fallthrough(const ast::BlockStatement* stmts) {
  if (stmts->empty()) {
    return false;
  }

  return stmts->last()->Is<ast::BreakStatement>() ||
         stmts->last()->Is<ast::FallthroughStatement>();
}

const char* image_format_to_rwtexture_type(ast::ImageFormat image_format) {
  switch (image_format) {
    case ast::ImageFormat::kRgba8Unorm:
    case ast::ImageFormat::kRgba8Snorm:
    case ast::ImageFormat::kRgba16Float:
    case ast::ImageFormat::kR32Float:
    case ast::ImageFormat::kRg32Float:
    case ast::ImageFormat::kRgba32Float:
      return "float4";
    case ast::ImageFormat::kRgba8Uint:
    case ast::ImageFormat::kRgba16Uint:
    case ast::ImageFormat::kR32Uint:
    case ast::ImageFormat::kRg32Uint:
    case ast::ImageFormat::kRgba32Uint:
      return "uint4";
    case ast::ImageFormat::kRgba8Sint:
    case ast::ImageFormat::kRgba16Sint:
    case ast::ImageFormat::kR32Sint:
    case ast::ImageFormat::kRg32Sint:
    case ast::ImageFormat::kRgba32Sint:
      return "int4";
    default:
      return nullptr;
  }
}

// Helper for writing " : register(RX, spaceY)", where R is the register, X is
// the binding point binding value, and Y is the binding point group value.
struct RegisterAndSpace {
  RegisterAndSpace(char r, ast::Variable::BindingPoint bp)
      : reg(r), binding_point(bp) {}

  char const reg;
  ast::Variable::BindingPoint const binding_point;
};

std::ostream& operator<<(std::ostream& s, const RegisterAndSpace& rs) {
  s << " : register(" << rs.reg << rs.binding_point.binding->value()
    << ", space" << rs.binding_point.group->value() << ")";
  return s;
}

}  // namespace

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  if (!builder_.HasTransformApplied<transform::Hlsl>()) {
    diagnostics_.add_error(
        diag::System::Writer,
        "HLSL writer requires the transform::Hlsl sanitizer to have been "
        "applied to the input program");
    return false;
  }

  const TypeInfo* last_kind = nullptr;
  size_t last_padding_line = 0;

  if (!FindAndEmitVectorAssignmentInLoopFunctions()) {
    return false;
  }

  for (auto* decl : builder_.AST().GlobalDeclarations()) {
    if (decl->Is<ast::Alias>()) {
      continue;  // Ignore aliases.
    }

    // Emit a new line between declarations if the type of declaration has
    // changed, or we're about to emit a function
    auto* kind = &decl->TypeInfo();
    if (current_buffer_->lines.size() != last_padding_line) {
      if (last_kind && (last_kind != kind || decl->Is<ast::Function>())) {
        line();
        last_padding_line = current_buffer_->lines.size();
      }
    }
    last_kind = kind;

    if (auto* global = decl->As<ast::Variable>()) {
      if (!EmitGlobalVariable(global)) {
        return false;
      }
    } else if (auto* str = decl->As<ast::Struct>()) {
      if (!EmitStructType(builder_.Sem().Get(str))) {
        return false;
      }
    } else if (auto* func = decl->As<ast::Function>()) {
      if (func->IsEntryPoint()) {
        if (!EmitEntryPointFunction(func)) {
          return false;
        }
      } else {
        if (!EmitFunction(func)) {
          return false;
        }
      }
    } else {
      TINT_ICE(Writer, diagnostics_)
          << "unhandled module-scope declaration: " << decl->TypeInfo().name;
      return false;
    }
  }

  return true;
}

bool GeneratorImpl::FindAndEmitVectorAssignmentInLoopFunctions() {
  auto is_in_loop = [](const sem::Expression* expr) {
    auto* block = expr->Stmt()->Block();
    if (!block) {
      return false;
    }
    return block->FindFirstParent<sem::LoopBlockStatement>() != nullptr;
  };

  auto emit_function_once = [&](const sem::Vector* vec) {
    utils::GetOrCreate(vector_assignment_in_loop_funcs_, vec, [&] {
      std::ostringstream ss;
      EmitType(ss, vec, tint::ast::StorageClass::kInvalid,
               ast::Access::kUndefined, "");
      auto func_name = UniqueIdentifier("Set_" + ss.str());
      {
        auto out = line();
        out << "void " << func_name << "(inout ";
        EmitType(out, vec, ast::StorageClass::kInvalid, ast::Access::kUndefined,
                 "");
        out << " vec, int idx, ";
        EmitType(out, vec->type(), ast::StorageClass::kInvalid,
                 ast::Access::kUndefined, "");
        out << " val) {";
      }
      {
        ScopedIndent si(this);
        line() << "switch(idx) {";
        {
          ScopedIndent si2(this);
          for (size_t i = 0; i < vec->size(); ++i) {
            auto sidx = std::to_string(i);
            line() << "case " + sidx + ": vec[" + sidx + "] = val; break;";
          }
        }
        line() << "}";
      }
      line() << "}";
      return func_name;
    });
  };

  // Find vector assignments via an accessor expression (index) within loops so
  // that we can replace them later with calls to setter functions. Also emit
  // the setter functions per vector type as we find them. We do this to avoid
  // having FCX fail to unroll loops with "error X3511: forced to unroll loop,
  // but unrolling failed." See crbug.com/tint/534.

  for (auto* ast_node : program_->ASTNodes().Objects()) {
    auto* ast_assign = ast_node->As<ast::AssignmentStatement>();
    if (!ast_assign) {
      continue;
    }

    auto* ast_access_expr =
        ast_assign->lhs()->As<ast::ArrayAccessorExpression>();
    if (!ast_access_expr) {
      continue;
    }

    auto* array_expr = builder_.Sem().Get(ast_access_expr->array());
    auto* vec = array_expr->Type()->UnwrapRef()->As<sem::Vector>();

    // Skip non-vectors
    if (!vec) {
      continue;
    }

    // Skip if not part of a loop
    if (!is_in_loop(array_expr)) {
      continue;
    }

    // Save this assignment along with the vector type
    vector_assignments_in_loops_.emplace(ast_assign, vec);

    // Emit the function if it hasn't already
    emit_function_once(vec);
  }

  return true;
}

bool GeneratorImpl::EmitVectorAssignmentInLoopCall(
    const ast::AssignmentStatement* stmt,
    const sem::Vector* vec) {
  auto* ast_access_expr = stmt->lhs()->As<ast::ArrayAccessorExpression>();

  auto out = line();
  out << vector_assignment_in_loop_funcs_.at(vec) << "(";
  if (!EmitExpression(out, ast_access_expr->array())) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, ast_access_expr->idx_expr())) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, stmt->rhs())) {
    return false;
  }
  out << ");";

  return true;
}

bool GeneratorImpl::EmitArrayAccessor(std::ostream& out,
                                      ast::ArrayAccessorExpression* expr) {
  if (!EmitExpression(out, expr->array())) {
    return false;
  }
  out << "[";

  if (!EmitExpression(out, expr->idx_expr())) {
    return false;
  }
  out << "]";

  return true;
}

bool GeneratorImpl::EmitBitcast(std::ostream& out,
                                ast::BitcastExpression* expr) {
  auto* type = TypeOf(expr);
  if (!type->is_integer_scalar() && !type->is_float_scalar()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Unable to do bitcast to type " + type->type_name());
    return false;
  }

  out << "as";
  if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                "")) {
    return false;
  }
  out << "(";
  if (!EmitExpression(out, expr->expr())) {
    return false;
  }
  out << ")";
  return true;
}

bool GeneratorImpl::EmitAssign(ast::AssignmentStatement* stmt) {
  auto iter = vector_assignments_in_loops_.find(stmt);
  if (iter != vector_assignments_in_loops_.end()) {
    return EmitVectorAssignmentInLoopCall(iter->first, iter->second);
  }

  auto out = line();
  if (!EmitExpression(out, stmt->lhs())) {
    return false;
  }
  out << " = ";
  if (!EmitExpression(out, stmt->rhs())) {
    return false;
  }
  out << ";";
  return true;
}

bool GeneratorImpl::EmitBinary(std::ostream& out, ast::BinaryExpression* expr) {
  if (expr->op() == ast::BinaryOp::kLogicalAnd ||
      expr->op() == ast::BinaryOp::kLogicalOr) {
    auto name = UniqueIdentifier(kTempNamePrefix);

    {
      auto pre = line();
      pre << "bool " << name << " = ";
      if (!EmitExpression(pre, expr->lhs())) {
        return false;
      }
      pre << ";";
    }

    if (expr->op() == ast::BinaryOp::kLogicalOr) {
      line() << "if (!" << name << ") {";
    } else {
      line() << "if (" << name << ") {";
    }

    {
      ScopedIndent si(this);
      auto pre = line();
      pre << name << " = ";
      if (!EmitExpression(pre, expr->rhs())) {
        return false;
      }
      pre << ";";
    }

    line() << "}";

    out << "(" << name << ")";
    return true;
  }

  auto* lhs_type = TypeOf(expr->lhs())->UnwrapRef();
  auto* rhs_type = TypeOf(expr->rhs())->UnwrapRef();
  // Multiplying by a matrix requires the use of `mul` in order to get the
  // type of multiply we desire.
  if (expr->op() == ast::BinaryOp::kMultiply &&
      ((lhs_type->Is<sem::Vector>() && rhs_type->Is<sem::Matrix>()) ||
       (lhs_type->Is<sem::Matrix>() && rhs_type->Is<sem::Vector>()) ||
       (lhs_type->Is<sem::Matrix>() && rhs_type->Is<sem::Matrix>()))) {
    // Matrices are transposed, so swap LHS and RHS.
    out << "mul(";
    if (!EmitExpression(out, expr->rhs())) {
      return false;
    }
    out << ", ";
    if (!EmitExpression(out, expr->lhs())) {
      return false;
    }
    out << ")";

    return true;
  }

  out << "(";
  if (!EmitExpression(out, expr->lhs())) {
    return false;
  }
  out << " ";

  switch (expr->op()) {
    case ast::BinaryOp::kAnd:
      out << "&";
      break;
    case ast::BinaryOp::kOr:
      out << "|";
      break;
    case ast::BinaryOp::kXor:
      out << "^";
      break;
    case ast::BinaryOp::kLogicalAnd:
    case ast::BinaryOp::kLogicalOr: {
      // These are both handled above.
      TINT_UNREACHABLE(Writer, diagnostics_);
      return false;
    }
    case ast::BinaryOp::kEqual:
      out << "==";
      break;
    case ast::BinaryOp::kNotEqual:
      out << "!=";
      break;
    case ast::BinaryOp::kLessThan:
      out << "<";
      break;
    case ast::BinaryOp::kGreaterThan:
      out << ">";
      break;
    case ast::BinaryOp::kLessThanEqual:
      out << "<=";
      break;
    case ast::BinaryOp::kGreaterThanEqual:
      out << ">=";
      break;
    case ast::BinaryOp::kShiftLeft:
      out << "<<";
      break;
    case ast::BinaryOp::kShiftRight:
      // TODO(dsinclair): MSL is based on C++14, and >> in C++14 has
      // implementation-defined behaviour for negative LHS.  We may have to
      // generate extra code to implement WGSL-specified behaviour for negative
      // LHS.
      out << R"(>>)";
      break;

    case ast::BinaryOp::kAdd:
      out << "+";
      break;
    case ast::BinaryOp::kSubtract:
      out << "-";
      break;
    case ast::BinaryOp::kMultiply:
      out << "*";
      break;
    case ast::BinaryOp::kDivide:
      out << "/";
      break;
    case ast::BinaryOp::kModulo:
      out << "%";
      break;
    case ast::BinaryOp::kNone:
      diagnostics_.add_error(diag::System::Writer,
                             "missing binary operation type");
      return false;
  }
  out << " ";

  if (!EmitExpression(out, expr->rhs())) {
    return false;
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitStatements(const ast::StatementList& stmts) {
  for (auto* s : stmts) {
    if (!EmitStatement(s)) {
      return false;
    }
  }
  return true;
}

bool GeneratorImpl::EmitStatementsWithIndent(const ast::StatementList& stmts) {
  ScopedIndent si(this);
  return EmitStatements(stmts);
}

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
  line() << "{";
  if (!EmitStatementsWithIndent(stmt->statements())) {
    return false;
  }
  line() << "}";
  return true;
}

bool GeneratorImpl::EmitBreak(ast::BreakStatement*) {
  line() << "break;";
  return true;
}

bool GeneratorImpl::EmitCall(std::ostream& out, ast::CallExpression* expr) {
  const auto& params = expr->params();
  auto* ident = expr->func();
  auto* call = builder_.Sem().Get(expr);
  auto* target = call->Target();

  if (auto* func = target->As<sem::Function>()) {
    if (ast::HasDecoration<
            transform::CalculateArrayLength::BufferSizeIntrinsic>(
            func->Declaration()->decorations())) {
      // Special function generated by the CalculateArrayLength transform for
      // calling X.GetDimensions(Y)
      if (!EmitExpression(out, params[0])) {
        return false;
      }
      out << ".GetDimensions(";
      if (!EmitExpression(out, params[1])) {
        return false;
      }
      out << ")";
      return true;
    }

    if (auto* intrinsic =
            ast::GetDecoration<transform::DecomposeMemoryAccess::Intrinsic>(
                func->Declaration()->decorations())) {
      switch (intrinsic->storage_class) {
        case ast::StorageClass::kUniform:
          return EmitUniformBufferAccess(out, expr, intrinsic);
        case ast::StorageClass::kStorage:
          return EmitStorageBufferAccess(out, expr, intrinsic);
        default:
          TINT_UNREACHABLE(Writer, diagnostics_)
              << "unsupported DecomposeMemoryAccess::Intrinsic storage class:"
              << intrinsic->storage_class;
          return false;
      }
    }
  }

  if (auto* intrinsic = call->Target()->As<sem::Intrinsic>()) {
    if (intrinsic->IsTexture()) {
      return EmitTextureCall(out, expr, intrinsic);
    } else if (intrinsic->Type() == sem::IntrinsicType::kSelect) {
      return EmitSelectCall(out, expr);
    } else if (intrinsic->Type() == sem::IntrinsicType::kFrexp) {
      return EmitFrexpCall(out, expr, intrinsic);
    } else if (intrinsic->Type() == sem::IntrinsicType::kIsNormal) {
      return EmitIsNormalCall(out, expr, intrinsic);
    } else if (intrinsic->Type() == sem::IntrinsicType::kIgnore) {
      return EmitExpression(out, expr->params()[0]);
    } else if (intrinsic->IsDataPacking()) {
      return EmitDataPackingCall(out, expr, intrinsic);
    } else if (intrinsic->IsDataUnpacking()) {
      return EmitDataUnpackingCall(out, expr, intrinsic);
    } else if (intrinsic->IsBarrier()) {
      return EmitBarrierCall(out, intrinsic);
    } else if (intrinsic->IsAtomic()) {
      return EmitWorkgroupAtomicCall(out, expr, intrinsic);
    }
    auto name = generate_builtin_name(intrinsic);
    if (name.empty()) {
      return false;
    }

    out << name << "(";

    bool first = true;
    for (auto* param : params) {
      if (!first) {
        out << ", ";
      }
      first = false;

      if (!EmitExpression(out, param)) {
        return false;
      }
    }

    out << ")";
    return true;
  }

  auto name = builder_.Symbols().NameFor(ident->symbol());
  auto caller_sym = ident->symbol();

  auto* func = builder_.AST().Functions().Find(ident->symbol());
  if (func == nullptr) {
    diagnostics_.add_error(diag::System::Writer,
                           "Unable to find function: " +
                               builder_.Symbols().NameFor(ident->symbol()));
    return false;
  }

  out << name << "(";

  bool first = true;
  for (auto* param : params) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(out, param)) {
      return false;
    }
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitUniformBufferAccess(
    std::ostream& out,
    ast::CallExpression* expr,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
  const auto& params = expr->params();

  std::string scalar_offset = UniqueIdentifier("scalar_offset");
  {
    auto pre = line();
    pre << "const uint " << scalar_offset << " = (";
    if (!EmitExpression(pre, params[1])) {  // offset
      return false;
    }
    pre << ") / 4;";
  }

  using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;
  using DataType = transform::DecomposeMemoryAccess::Intrinsic::DataType;
  switch (intrinsic->op) {
    case Op::kLoad: {
      auto cast = [&](const char* to, auto&& load) {
        out << to << "(";
        auto result = load();
        out << ")";
        return result;
      };
      auto load_scalar = [&]() {
        if (!EmitExpression(out, params[0])) {  // buffer
          return false;
        }
        out << "[" << scalar_offset << " / 4][" << scalar_offset << " % 4]";
        return true;
      };
      // Has a minimum alignment of 8 bytes, so is either .xy or .zw
      auto load_vec2 = [&] {
        std::string ubo_load = UniqueIdentifier("ubo_load");

        {
          auto pre = line();
          pre << "uint4 " << ubo_load << " = ";
          if (!EmitExpression(pre, params[0])) {  // buffer
            return false;
          }
          pre << "[" << scalar_offset << " / 4];";
        }

        out << "((" << scalar_offset << " & 2) ? " << ubo_load
            << ".zw : " << ubo_load << ".xy)";
        return true;
      };
      // vec3 has a minimum alignment of 16 bytes, so is just a .xyz swizzle
      auto load_vec3 = [&] {
        if (!EmitExpression(out, params[0])) {  // buffer
          return false;
        }
        out << "[" << scalar_offset << " / 4].xyz";
        return true;
      };
      // vec4 has a minimum alignment of 16 bytes, easiest case
      auto load_vec4 = [&] {
        if (!EmitExpression(out, params[0])) {  // buffer
          return false;
        }
        out << "[" << scalar_offset << " / 4]";
        return true;
      };
      switch (intrinsic->type) {
        case DataType::kU32:
          return load_scalar();
        case DataType::kF32:
          return cast("asfloat", load_scalar);
        case DataType::kI32:
          return cast("asint", load_scalar);
        case DataType::kVec2U32:
          return load_vec2();
        case DataType::kVec2F32:
          return cast("asfloat", load_vec2);
        case DataType::kVec2I32:
          return cast("asint", load_vec2);
        case DataType::kVec3U32:
          return load_vec3();
        case DataType::kVec3F32:
          return cast("asfloat", load_vec3);
        case DataType::kVec3I32:
          return cast("asint", load_vec3);
        case DataType::kVec4U32:
          return load_vec4();
        case DataType::kVec4F32:
          return cast("asfloat", load_vec4);
        case DataType::kVec4I32:
          return cast("asint", load_vec4);
      }
      TINT_UNREACHABLE(Writer, diagnostics_)
          << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
          << static_cast<int>(intrinsic->type);
      return false;
    }
    default:
      break;
  }
  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported DecomposeMemoryAccess::Intrinsic::Op: "
      << static_cast<int>(intrinsic->op);
  return false;
}

bool GeneratorImpl::EmitStorageBufferAccess(
    std::ostream& out,
    ast::CallExpression* expr,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
  const auto& params = expr->params();

  using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;
  using DataType = transform::DecomposeMemoryAccess::Intrinsic::DataType;
  switch (intrinsic->op) {
    case Op::kLoad: {
      auto load = [&](const char* cast, int n) {
        if (cast) {
          out << cast << "(";
        }
        if (!EmitExpression(out, params[0])) {  // buffer
          return false;
        }
        out << ".Load";
        if (n > 1) {
          out << n;
        }
        ScopedParen sp(out);
        if (!EmitExpression(out, params[1])) {  // offset
          return false;
        }
        if (cast) {
          out << ")";
        }
        return true;
      };
      switch (intrinsic->type) {
        case DataType::kU32:
          return load(nullptr, 1);
        case DataType::kF32:
          return load("asfloat", 1);
        case DataType::kI32:
          return load("asint", 1);
        case DataType::kVec2U32:
          return load(nullptr, 2);
        case DataType::kVec2F32:
          return load("asfloat", 2);
        case DataType::kVec2I32:
          return load("asint", 2);
        case DataType::kVec3U32:
          return load(nullptr, 3);
        case DataType::kVec3F32:
          return load("asfloat", 3);
        case DataType::kVec3I32:
          return load("asint", 3);
        case DataType::kVec4U32:
          return load(nullptr, 4);
        case DataType::kVec4F32:
          return load("asfloat", 4);
        case DataType::kVec4I32:
          return load("asint", 4);
      }
      TINT_UNREACHABLE(Writer, diagnostics_)
          << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
          << static_cast<int>(intrinsic->type);
      return false;
    }

    case Op::kStore: {
      auto store = [&](int n) {
        if (!EmitExpression(out, params[0])) {  // buffer
          return false;
        }
        out << ".Store";
        if (n > 1) {
          out << n;
        }
        ScopedParen sp1(out);
        if (!EmitExpression(out, params[1])) {  // offset
          return false;
        }
        out << ", asuint";
        ScopedParen sp2(out);
        if (!EmitExpression(out, params[2])) {  // value
          return false;
        }
        return true;
      };
      switch (intrinsic->type) {
        case DataType::kU32:
          return store(1);
        case DataType::kF32:
          return store(1);
        case DataType::kI32:
          return store(1);
        case DataType::kVec2U32:
          return store(2);
        case DataType::kVec2F32:
          return store(2);
        case DataType::kVec2I32:
          return store(2);
        case DataType::kVec3U32:
          return store(3);
        case DataType::kVec3F32:
          return store(3);
        case DataType::kVec3I32:
          return store(3);
        case DataType::kVec4U32:
          return store(4);
        case DataType::kVec4F32:
          return store(4);
        case DataType::kVec4I32:
          return store(4);
      }
      TINT_UNREACHABLE(Writer, diagnostics_)
          << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
          << static_cast<int>(intrinsic->type);
      return false;
    }

    case Op::kAtomicLoad:
    case Op::kAtomicStore:
    case Op::kAtomicAdd:
    case Op::kAtomicMax:
    case Op::kAtomicMin:
    case Op::kAtomicAnd:
    case Op::kAtomicOr:
    case Op::kAtomicXor:
    case Op::kAtomicExchange:
    case Op::kAtomicCompareExchangeWeak:
      return EmitStorageAtomicCall(out, expr, intrinsic->op);
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported DecomposeMemoryAccess::Intrinsic::Op: "
      << static_cast<int>(intrinsic->op);
  return false;
}

bool GeneratorImpl::EmitStorageAtomicCall(
    std::ostream& out,
    ast::CallExpression* expr,
    transform::DecomposeMemoryAccess::Intrinsic::Op op) {
  using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;

  std::string result = UniqueIdentifier("atomic_result");

  auto* result_ty = TypeOf(expr);
  if (!result_ty->Is<sem::Void>()) {
    auto pre = line();
    if (!EmitTypeAndName(pre, TypeOf(expr), ast::StorageClass::kNone,
                         ast::Access::kUndefined, result)) {
      return false;
    }
    pre << " = ";
    if (!EmitZeroValue(pre, result_ty)) {
      return false;
    }
    pre << ";";
  }

  auto* buffer = expr->params()[0];
  auto* offset = expr->params()[1];

  auto call_buffer_method = [&](const char* name) {
    auto pre = line();
    if (!EmitExpression(pre, buffer)) {
      return false;
    }
    pre << "." << name;
    {
      ScopedParen sp(pre);
      if (!EmitExpression(pre, offset)) {
        return false;
      }

      for (size_t i = 1; i < expr->params().size() - 1; i++) {
        auto* arg = expr->params()[i];
        pre << ", ";
        if (!EmitExpression(pre, arg)) {
          return false;
        }
      }

      pre << ", " << result;
    }
    pre << ";";

    out << result;
    return true;
  };

  switch (op) {
    case Op::kAtomicLoad: {
      // HLSL does not have an InterlockedLoad, so we emulate it with
      // InterlockedOr using 0 as the OR value
      auto pre = line();
      if (!EmitExpression(pre, buffer)) {
        return false;
      }
      pre << ".InterlockedOr";
      {
        ScopedParen sp(pre);
        if (!EmitExpression(pre, offset)) {
          return false;
        }
        pre << ", 0, " << result;
      }

      pre << ";";
      out << result;
      return true;
    }
    case Op::kAtomicStore: {
      // HLSL does not have an InterlockedStore, so we emulate it with
      // InterlockedExchange and discard the returned value
      auto pre = line();
      auto* value = expr->params()[2];
      auto* value_ty = TypeOf(value);
      if (!EmitTypeAndName(pre, value_ty, ast::StorageClass::kNone,
                           ast::Access::kUndefined, result)) {
        return false;
      }
      pre << " = ";
      if (!EmitZeroValue(pre, value_ty)) {
        return false;
      }
      pre << ";";

      if (!EmitExpression(out, buffer)) {
        return false;
      }
      out << ".InterlockedExchange";
      {
        ScopedParen sp(out);
        if (!EmitExpression(out, offset)) {
          return false;
        }
        out << ", ";
        if (!EmitExpression(out, value)) {
          return false;
        }
        out << ", " << result;
      }
      return true;
    }
    case Op::kAtomicCompareExchangeWeak: {
      auto* compare_value = expr->params()[2];
      auto* value = expr->params()[3];

      std::string compare = UniqueIdentifier("atomic_compare_value");
      {  // T atomic_compare_value = compare_value;
        auto pre = line();
        if (!EmitTypeAndName(pre, TypeOf(compare_value),
                             ast::StorageClass::kNone, ast::Access::kUndefined,
                             compare)) {
          return false;
        }
        pre << " = ";
        if (!EmitExpression(pre, compare_value)) {
          return false;
        }
        pre << ";";
      }
      {  // buffer.InterlockedCompareExchange(offset, compare, value, result.x);
        auto pre = line();
        if (!EmitExpression(pre, buffer)) {
          return false;
        }
        pre << ".InterlockedCompareExchange";
        {
          ScopedParen sp(pre);
          if (!EmitExpression(pre, offset)) {
            return false;
          }
          pre << ", " << compare << ", ";
          if (!EmitExpression(pre, value)) {
            return false;
          }
          pre << ", " << result << ".x";
        }
        pre << ";";
      }
      {  // result.y = result.x == compare;
        line() << result << ".y = " << result << ".x == " << compare << ";";
      }

      out << result;
      return true;
    }

    case Op::kAtomicAdd:
      return call_buffer_method("InterlockedAdd");

    case Op::kAtomicMax:
      return call_buffer_method("InterlockedMax");

    case Op::kAtomicMin:
      return call_buffer_method("InterlockedMin");

    case Op::kAtomicAnd:
      return call_buffer_method("InterlockedAnd");

    case Op::kAtomicOr:
      return call_buffer_method("InterlockedOr");

    case Op::kAtomicXor:
      return call_buffer_method("InterlockedXor");

    case Op::kAtomicExchange:
      return call_buffer_method("InterlockedExchange");

    default:
      break;
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported atomic DecomposeMemoryAccess::Intrinsic::Op: "
      << static_cast<int>(op);
  return false;
}

bool GeneratorImpl::EmitWorkgroupAtomicCall(std::ostream& out,
                                            ast::CallExpression* expr,
                                            const sem::Intrinsic* intrinsic) {
  std::string result = UniqueIdentifier("atomic_result");

  if (!intrinsic->ReturnType()->Is<sem::Void>()) {
    auto pre = line();
    if (!EmitTypeAndName(pre, intrinsic->ReturnType(), ast::StorageClass::kNone,
                         ast::Access::kUndefined, result)) {
      return false;
    }
    pre << " = ";
    if (!EmitZeroValue(pre, intrinsic->ReturnType())) {
      return false;
    }
    pre << ";";
  }

  auto call = [&](const char* name) {
    auto pre = line();
    pre << name;

    {
      ScopedParen sp(pre);
      for (size_t i = 0; i < expr->params().size(); i++) {
        auto* arg = expr->params()[i];
        if (i > 0) {
          pre << ", ";
        }
        if (!EmitExpression(pre, arg)) {
          return false;
        }
      }

      pre << ", " << result;
    }

    pre << ";";

    out << result;
    return true;
  };

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kAtomicLoad: {
      // HLSL does not have an InterlockedLoad, so we emulate it with
      // InterlockedOr using 0 as the OR value
      auto pre = line();
      pre << "InterlockedOr";
      {
        ScopedParen sp(pre);
        if (!EmitExpression(pre, expr->params()[0])) {
          return false;
        }
        pre << ", 0, " << result;
      }
      pre << ";";

      out << result;
      return true;
    }
    case sem::IntrinsicType::kAtomicStore: {
      // HLSL does not have an InterlockedStore, so we emulate it with
      // InterlockedExchange and discard the returned value
      {  // T result = 0;
        auto pre = line();
        auto* value_ty = intrinsic->Parameters()[1].type;
        if (!EmitTypeAndName(pre, value_ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, result)) {
          return false;
        }
        pre << " = ";
        if (!EmitZeroValue(pre, value_ty)) {
          return false;
        }
        pre << ";";
      }

      out << "InterlockedExchange";
      {
        ScopedParen sp(out);
        if (!EmitExpression(out, expr->params()[0])) {
          return false;
        }
        out << ", ";
        if (!EmitExpression(out, expr->params()[1])) {
          return false;
        }
        out << ", " << result;
      }
      return true;
    }
    case sem::IntrinsicType::kAtomicCompareExchangeWeak: {
      auto* dest = expr->params()[0];
      auto* compare_value = expr->params()[1];
      auto* value = expr->params()[2];

      std::string compare = UniqueIdentifier("atomic_compare_value");

      {  // T compare_value = <compare_value>;
        auto pre = line();
        if (!EmitTypeAndName(pre, TypeOf(compare_value),
                             ast::StorageClass::kNone, ast::Access::kUndefined,
                             compare)) {
          return false;
        }
        pre << " = ";
        if (!EmitExpression(pre, compare_value)) {
          return false;
        }
        pre << ";";
      }

      {  // InterlockedCompareExchange(dst, compare, value, result.x);
        auto pre = line();
        pre << "InterlockedCompareExchange";
        {
          ScopedParen sp(pre);
          if (!EmitExpression(pre, dest)) {
            return false;
          }
          pre << ", " << compare << ", ";
          if (!EmitExpression(pre, value)) {
            return false;
          }
          pre << ", " << result << ".x";
        }
        pre << ";";
      }

      {  // result.y = result.x == compare;
        line() << result << ".y = " << result << ".x == " << compare << ";";
      }

      out << result;
      return true;
    }

    case sem::IntrinsicType::kAtomicAdd:
      return call("InterlockedAdd");

    case sem::IntrinsicType::kAtomicMax:
      return call("InterlockedMax");

    case sem::IntrinsicType::kAtomicMin:
      return call("InterlockedMin");

    case sem::IntrinsicType::kAtomicAnd:
      return call("InterlockedAnd");

    case sem::IntrinsicType::kAtomicOr:
      return call("InterlockedOr");

    case sem::IntrinsicType::kAtomicXor:
      return call("InterlockedXor");

    case sem::IntrinsicType::kAtomicExchange:
      return call("InterlockedExchange");

    default:
      break;
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported atomic intrinsic: " << intrinsic->Type();
  return false;
}

bool GeneratorImpl::EmitSelectCall(std::ostream& out,
                                   ast::CallExpression* expr) {
  auto* expr_true = expr->params()[0];
  auto* expr_false = expr->params()[1];
  auto* expr_cond = expr->params()[2];
  ScopedParen paren(out);
  if (!EmitExpression(out, expr_cond)) {
    return false;
  }

  out << " ? ";

  if (!EmitExpression(out, expr_true)) {
    return false;
  }

  out << " : ";

  if (!EmitExpression(out, expr_false)) {
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitFrexpCall(std::ostream& out,
                                  ast::CallExpression* expr,
                                  const sem::Intrinsic* intrinsic) {
  // Exponent is an integer in WGSL, but HLSL wants a float.
  // We need to make the call with a temporary float, and then cast.

  auto signficand = intrinsic->Parameters()[0];
  auto exponent = intrinsic->Parameters()[1];

  std::string width;
  if (auto* vec = signficand.type->As<sem::Vector>()) {
    width = std::to_string(vec->size());
  }

  // Exponent is an integer, which HLSL does not have an overload for.
  // We need to cast from a float.
  auto float_exp = UniqueIdentifier(kTempNamePrefix);
  auto significand = UniqueIdentifier(kTempNamePrefix);
  line() << "float" << width << " " << float_exp << ";";
  {
    auto pre = line();
    pre << "float" << width << " " << significand << " = frexp(";
    if (!EmitExpression(pre, expr->params()[0])) {
      return false;
    }
    pre << ", " << float_exp << ");";
  }
  {
    auto pre = line();
    if (!EmitExpression(pre, expr->params()[1])) {
      return false;
    }
    pre << " = ";
    if (!EmitType(pre, exponent.type->UnwrapPtr(), ast::StorageClass::kNone,
                  ast::Access::kUndefined, "")) {
      return false;
    }
    pre << "(" << float_exp << ");";
  }

  out << significand;
  return true;
}

bool GeneratorImpl::EmitIsNormalCall(std::ostream& out,
                                     ast::CallExpression* expr,
                                     const sem::Intrinsic* intrinsic) {
  // HLSL doesn't have a isNormal intrinsic, we need to emulate
  auto input = intrinsic->Parameters()[0];

  std::string width;
  if (auto* vec = input.type->As<sem::Vector>()) {
    width = std::to_string(vec->size());
  }

  constexpr auto* kExponentMask = "0x7f80000";
  constexpr auto* kMinNormalExponent = "0x0080000";
  constexpr auto* kMaxNormalExponent = "0x7f00000";

  auto exponent = UniqueIdentifier("tint_isnormal_exponent");
  auto clamped = UniqueIdentifier("tint_isnormal_clamped");

  {
    auto pre = line();
    pre << "uint" << width << " " << exponent << " = asuint(";
    if (!EmitExpression(pre, expr->params()[0])) {
      return false;
    }
    pre << ") & " << kExponentMask << ";";
  }
  line() << "uint" << width << " " << clamped << " = "
         << "clamp(" << exponent << ", " << kMinNormalExponent << ", "
         << kMaxNormalExponent << ");";

  out << "(" << clamped << " == " << exponent << ")";
  return true;
}

bool GeneratorImpl::EmitDataPackingCall(std::ostream& out,
                                        ast::CallExpression* expr,
                                        const sem::Intrinsic* intrinsic) {
  auto* param = expr->params()[0];
  auto tmp_name = UniqueIdentifier(kTempNamePrefix);
  std::ostringstream expr_out;
  if (!EmitExpression(expr_out, param)) {
    return false;
  }
  uint32_t dims = 2;
  bool is_signed = false;
  uint32_t scale = 65535;
  if (intrinsic->Type() == sem::IntrinsicType::kPack4x8snorm ||
      intrinsic->Type() == sem::IntrinsicType::kPack4x8unorm) {
    dims = 4;
    scale = 255;
  }
  if (intrinsic->Type() == sem::IntrinsicType::kPack4x8snorm ||
      intrinsic->Type() == sem::IntrinsicType::kPack2x16snorm) {
    is_signed = true;
    scale = (scale - 1) / 2;
  }
  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kPack4x8snorm:
    case sem::IntrinsicType::kPack4x8unorm:
    case sem::IntrinsicType::kPack2x16snorm:
    case sem::IntrinsicType::kPack2x16unorm: {
      {
        auto pre = line();
        pre << (is_signed ? "" : "u") << "int" << dims << " " << tmp_name
            << " = " << (is_signed ? "" : "u") << "int" << dims
            << "(round(clamp(" << expr_out.str() << ", "
            << (is_signed ? "-1.0" : "0.0") << ", 1.0) * " << scale << ".0))";
        if (is_signed) {
          pre << " & " << (dims == 4 ? "0xff" : "0xffff");
        }
        pre << ";";
      }
      if (is_signed) {
        out << "asuint";
      }
      out << "(";
      out << tmp_name << ".x | " << tmp_name << ".y << " << (32 / dims);
      if (dims == 4) {
        out << " | " << tmp_name << ".z << 16 | " << tmp_name << ".w << 24";
      }
      out << ")";
      break;
    }
    case sem::IntrinsicType::kPack2x16float: {
      line() << "uint2 " << tmp_name << " = f32tof16(" << expr_out.str()
             << ");";
      out << "(" << tmp_name << ".x | " << tmp_name << ".y << 16)";
      break;
    }
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Internal error: unhandled data packing intrinsic");
      return false;
  }

  return true;
}

bool GeneratorImpl::EmitDataUnpackingCall(std::ostream& out,
                                          ast::CallExpression* expr,
                                          const sem::Intrinsic* intrinsic) {
  auto* param = expr->params()[0];
  auto tmp_name = UniqueIdentifier(kTempNamePrefix);
  std::ostringstream expr_out;
  if (!EmitExpression(expr_out, param)) {
    return false;
  }
  uint32_t dims = 2;
  bool is_signed = false;
  uint32_t scale = 65535;
  if (intrinsic->Type() == sem::IntrinsicType::kUnpack4x8snorm ||
      intrinsic->Type() == sem::IntrinsicType::kUnpack4x8unorm) {
    dims = 4;
    scale = 255;
  }
  if (intrinsic->Type() == sem::IntrinsicType::kUnpack4x8snorm ||
      intrinsic->Type() == sem::IntrinsicType::kUnpack2x16snorm) {
    is_signed = true;
    scale = (scale - 1) / 2;
  }
  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kUnpack4x8snorm:
    case sem::IntrinsicType::kUnpack2x16snorm: {
      auto tmp_name2 = UniqueIdentifier(kTempNamePrefix);
      line() << "int " << tmp_name2 << " = int(" << expr_out.str() << ");";
      {  // Perform sign extension on the converted values.
        auto pre = line();
        pre << "int" << dims << " " << tmp_name << " = int" << dims << "(";
        if (dims == 2) {
          pre << tmp_name2 << " << 16, " << tmp_name2 << ") >> 16";
        } else {
          pre << tmp_name2 << " << 24, " << tmp_name2 << " << 16, " << tmp_name2
              << " << 8, " << tmp_name2 << ") >> 24";
        }
        pre << ";";
      }

      out << "clamp(float" << dims << "(" << tmp_name << ") / " << scale
          << ".0, " << (is_signed ? "-1.0" : "0.0") << ", 1.0)";
      break;
    }
    case sem::IntrinsicType::kUnpack4x8unorm:
    case sem::IntrinsicType::kUnpack2x16unorm: {
      auto tmp_name2 = UniqueIdentifier(kTempNamePrefix);
      line() << "uint " << tmp_name2 << " = " << expr_out.str() << ";";
      {
        auto pre = line();
        pre << "uint" << dims << " " << tmp_name << " = uint" << dims << "(";
        pre << tmp_name2 << " & " << (dims == 2 ? "0xffff" : "0xff") << ", ";
        if (dims == 4) {
          pre << "(" << tmp_name2 << " >> " << (32 / dims) << ") & 0xff, ("
              << tmp_name2 << " >> 16) & 0xff, " << tmp_name2 << " >> 24";
        } else {
          pre << tmp_name2 << " >> " << (32 / dims);
        }
        pre << ");";
      }
      out << "float" << dims << "(" << tmp_name << ") / " << scale << ".0";
      break;
    }
    case sem::IntrinsicType::kUnpack2x16float:
      line() << "uint " << tmp_name << " = " << expr_out.str() << ";";
      out << "f16tof32(uint2(" << tmp_name << " & 0xffff, " << tmp_name
          << " >> 16))";
      break;
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Internal error: unhandled data packing intrinsic");
      return false;
  }

  return true;
}

bool GeneratorImpl::EmitBarrierCall(std::ostream& out,
                                    const sem::Intrinsic* intrinsic) {
  // TODO(crbug.com/tint/661): Combine sequential barriers to a single
  // instruction.
  if (intrinsic->Type() == sem::IntrinsicType::kWorkgroupBarrier) {
    out << "GroupMemoryBarrierWithGroupSync()";
  } else if (intrinsic->Type() == sem::IntrinsicType::kStorageBarrier) {
    out << "DeviceMemoryBarrierWithGroupSync()";
  } else {
    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unexpected barrier intrinsic type " << sem::str(intrinsic->Type());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitTextureCall(std::ostream& out,
                                    ast::CallExpression* expr,
                                    const sem::Intrinsic* intrinsic) {
  using Usage = sem::ParameterUsage;

  auto parameters = intrinsic->Parameters();
  auto arguments = expr->params();

  // Returns the argument with the given usage
  auto arg = [&](Usage usage) {
    int idx = sem::IndexOf(parameters, usage);
    return (idx >= 0) ? arguments[idx] : nullptr;
  };

  auto* texture = arg(Usage::kTexture);
  if (!texture) {
    TINT_ICE(Writer, diagnostics_) << "missing texture argument";
    return false;
  }

  auto* texture_type = TypeOf(texture)->UnwrapRef()->As<sem::Texture>();

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kTextureDimensions:
    case sem::IntrinsicType::kTextureNumLayers:
    case sem::IntrinsicType::kTextureNumLevels:
    case sem::IntrinsicType::kTextureNumSamples: {
      // All of these intrinsics use the GetDimensions() method on the texture
      bool is_ms = texture_type->Is<sem::MultisampledTexture>();
      int num_dimensions = 0;
      std::string swizzle;

      switch (intrinsic->Type()) {
        case sem::IntrinsicType::kTextureDimensions:
          switch (texture_type->dim()) {
            case ast::TextureDimension::kNone:
              TINT_ICE(Writer, diagnostics_) << "texture dimension is kNone";
              return false;
            case ast::TextureDimension::k1d:
              num_dimensions = 1;
              break;
            case ast::TextureDimension::k2d:
              num_dimensions = is_ms ? 3 : 2;
              swizzle = is_ms ? ".xy" : "";
              break;
            case ast::TextureDimension::k2dArray:
              num_dimensions = is_ms ? 4 : 3;
              swizzle = ".xy";
              break;
            case ast::TextureDimension::k3d:
              num_dimensions = 3;
              break;
            case ast::TextureDimension::kCube:
              num_dimensions = 2;
              break;
            case ast::TextureDimension::kCubeArray:
              num_dimensions = 3;
              swizzle = ".xy";
              break;
          }
          break;
        case sem::IntrinsicType::kTextureNumLayers:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(Writer, diagnostics_)
                  << "texture dimension is not arrayed";
              return false;
            case ast::TextureDimension::k2dArray:
              num_dimensions = is_ms ? 4 : 3;
              swizzle = ".z";
              break;
            case ast::TextureDimension::kCubeArray:
              num_dimensions = 3;
              swizzle = ".z";
              break;
          }
          break;
        case sem::IntrinsicType::kTextureNumLevels:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(Writer, diagnostics_)
                  << "texture dimension does not support mips";
              return false;
            case ast::TextureDimension::k2d:
            case ast::TextureDimension::kCube:
              num_dimensions = 3;
              swizzle = ".z";
              break;
            case ast::TextureDimension::k2dArray:
            case ast::TextureDimension::k3d:
            case ast::TextureDimension::kCubeArray:
              num_dimensions = 4;
              swizzle = ".w";
              break;
          }
          break;
        case sem::IntrinsicType::kTextureNumSamples:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(Writer, diagnostics_)
                  << "texture dimension does not support multisampling";
              return false;
            case ast::TextureDimension::k2d:
              num_dimensions = 3;
              swizzle = ".z";
              break;
            case ast::TextureDimension::k2dArray:
              num_dimensions = 4;
              swizzle = ".w";
              break;
          }
          break;
        default:
          TINT_ICE(Writer, diagnostics_) << "unexpected intrinsic";
          return false;
      }

      auto* level_arg = arg(Usage::kLevel);

      if (level_arg) {
        // `NumberOfLevels` is a non-optional argument if `MipLevel` was passed.
        // Increment the number of dimensions for the temporary vector to
        // accommodate this.
        num_dimensions++;

        // If the swizzle was empty, the expression will evaluate to the whole
        // vector. As we've grown the vector by one element, we now need to
        // swizzle to keep the result expression equivalent.
        if (swizzle.empty()) {
          static constexpr const char* swizzles[] = {"", ".x", ".xy", ".xyz"};
          swizzle = swizzles[num_dimensions - 1];
        }
      }

      if (num_dimensions > 4) {
        TINT_ICE(Writer, diagnostics_)
            << "Texture query intrinsic temporary vector has " << num_dimensions
            << " dimensions";
        return false;
      }

      // Declare a variable to hold the queried texture info
      auto dims = UniqueIdentifier(kTempNamePrefix);
      if (num_dimensions == 1) {
        line() << "int " << dims << ";";
      } else {
        line() << "int" << num_dimensions << " " << dims << ";";
      }

      {  // texture.GetDimensions(...)
        auto pre = line();
        if (!EmitExpression(pre, texture)) {
          return false;
        }
        pre << ".GetDimensions(";

        if (level_arg) {
          if (!EmitExpression(pre, level_arg)) {
            return false;
          }
          pre << ", ";
        } else if (intrinsic->Type() == sem::IntrinsicType::kTextureNumLevels) {
          pre << "0, ";
        }

        if (num_dimensions == 1) {
          pre << dims;
        } else {
          static constexpr char xyzw[] = {'x', 'y', 'z', 'w'};
          if (num_dimensions < 0 || num_dimensions > 4) {
            TINT_ICE(Writer, diagnostics_)
                << "vector dimensions are " << num_dimensions;
            return false;
          }
          for (int i = 0; i < num_dimensions; i++) {
            if (i > 0) {
              pre << ", ";
            }
            pre << dims << "." << xyzw[i];
          }
        }

        pre << ");";
      }

      // The out parameters of the GetDimensions() call is now in temporary
      // `dims` variable. This may be packed with other data, so the final
      // expression may require a swizzle.
      out << dims << swizzle;
      return true;
    }
    default:
      break;
  }

  if (!EmitExpression(out, texture))
    return false;

  // If pack_level_in_coords is true, then the mip level will be appended as the
  // last value of the coordinates argument. If the WGSL intrinsic overload does
  // not have a level parameter and pack_level_in_coords is true, then a zero
  // mip level will be inserted.
  bool pack_level_in_coords = false;

  uint32_t hlsl_ret_width = 4u;

  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kTextureSample:
      out << ".Sample(";
      break;
    case sem::IntrinsicType::kTextureSampleBias:
      out << ".SampleBias(";
      break;
    case sem::IntrinsicType::kTextureSampleLevel:
      out << ".SampleLevel(";
      break;
    case sem::IntrinsicType::kTextureSampleGrad:
      out << ".SampleGrad(";
      break;
    case sem::IntrinsicType::kTextureSampleCompare:
      out << ".SampleCmp(";
      hlsl_ret_width = 1;
      break;
    case sem::IntrinsicType::kTextureSampleCompareLevel:
      out << ".SampleCmpLevelZero(";
      hlsl_ret_width = 1;
      break;
    case sem::IntrinsicType::kTextureLoad:
      out << ".Load(";
      // Multisampled textures do not support mip-levels.
      if (!texture_type->Is<sem::MultisampledTexture>()) {
        pack_level_in_coords = true;
      }
      break;
    case sem::IntrinsicType::kTextureStore:
      out << "[";
      break;
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Internal compiler error: Unhandled texture intrinsic '" +
              std::string(intrinsic->str()) + "'");
      return false;
  }

  if (auto* sampler = arg(Usage::kSampler)) {
    if (!EmitExpression(out, sampler))
      return false;
    out << ", ";
  }

  auto* param_coords = arg(Usage::kCoords);
  if (!param_coords) {
    TINT_ICE(Writer, diagnostics_) << "missing coords argument";
    return false;
  }

  auto emit_vector_appended_with_i32_zero = [&](tint::ast::Expression* vector) {
    auto* i32 = builder_.create<sem::I32>();
    auto* zero = builder_.Expr(0);
    auto* stmt = builder_.Sem().Get(vector)->Stmt();
    builder_.Sem().Add(zero, builder_.create<sem::Expression>(zero, i32, stmt));
    auto* packed = AppendVector(&builder_, vector, zero);
    return EmitExpression(out, packed);
  };

  auto emit_vector_appended_with_level = [&](tint::ast::Expression* vector) {
    if (auto* level = arg(Usage::kLevel)) {
      auto* packed = AppendVector(&builder_, vector, level);
      return EmitExpression(out, packed);
    }
    return emit_vector_appended_with_i32_zero(vector);
  };

  if (auto* array_index = arg(Usage::kArrayIndex)) {
    // Array index needs to be appended to the coordinates.
    auto* packed = AppendVector(&builder_, param_coords, array_index);
    if (pack_level_in_coords) {
      // Then mip level needs to be appended to the coordinates.
      if (!emit_vector_appended_with_level(packed)) {
        return false;
      }
    } else {
      if (!EmitExpression(out, packed)) {
        return false;
      }
    }
  } else if (pack_level_in_coords) {
    // Mip level needs to be appended to the coordinates.
    if (!emit_vector_appended_with_level(param_coords)) {
      return false;
    }
  } else {
    if (!EmitExpression(out, param_coords)) {
      return false;
    }
  }

  for (auto usage : {Usage::kDepthRef, Usage::kBias, Usage::kLevel, Usage::kDdx,
                     Usage::kDdy, Usage::kSampleIndex, Usage::kOffset}) {
    if (usage == Usage::kLevel && pack_level_in_coords) {
      continue;  // mip level already packed in coordinates.
    }
    if (auto* e = arg(usage)) {
      out << ", ";
      if (!EmitExpression(out, e)) {
        return false;
      }
    }
  }

  if (intrinsic->Type() == sem::IntrinsicType::kTextureStore) {
    out << "] = ";
    if (!EmitExpression(out, arg(Usage::kValue))) {
      return false;
    }
  } else {
    out << ")";

    // If the intrinsic return type does not match the number of elements of the
    // HLSL intrinsic, we need to swizzle the expression to generate the correct
    // number of components.
    uint32_t wgsl_ret_width = 1;
    if (auto* vec = intrinsic->ReturnType()->As<sem::Vector>()) {
      wgsl_ret_width = vec->size();
    }
    if (wgsl_ret_width < hlsl_ret_width) {
      out << ".";
      for (uint32_t i = 0; i < wgsl_ret_width; i++) {
        out << "xyz"[i];
      }
    }
    if (wgsl_ret_width > hlsl_ret_width) {
      TINT_ICE(Writer, diagnostics_)
          << "WGSL return width (" << wgsl_ret_width
          << ") is wider than HLSL return width (" << hlsl_ret_width << ") for "
          << intrinsic->Type();
      return false;
    }
  }

  return true;
}

std::string GeneratorImpl::generate_builtin_name(
    const sem::Intrinsic* intrinsic) {
  switch (intrinsic->Type()) {
    case sem::IntrinsicType::kAbs:
    case sem::IntrinsicType::kAcos:
    case sem::IntrinsicType::kAll:
    case sem::IntrinsicType::kAny:
    case sem::IntrinsicType::kAsin:
    case sem::IntrinsicType::kAtan:
    case sem::IntrinsicType::kAtan2:
    case sem::IntrinsicType::kCeil:
    case sem::IntrinsicType::kClamp:
    case sem::IntrinsicType::kCos:
    case sem::IntrinsicType::kCosh:
    case sem::IntrinsicType::kCross:
    case sem::IntrinsicType::kDeterminant:
    case sem::IntrinsicType::kDistance:
    case sem::IntrinsicType::kDot:
    case sem::IntrinsicType::kExp:
    case sem::IntrinsicType::kExp2:
    case sem::IntrinsicType::kFloor:
    case sem::IntrinsicType::kFrexp:
    case sem::IntrinsicType::kLdexp:
    case sem::IntrinsicType::kLength:
    case sem::IntrinsicType::kLog:
    case sem::IntrinsicType::kLog2:
    case sem::IntrinsicType::kMax:
    case sem::IntrinsicType::kMin:
    case sem::IntrinsicType::kModf:
    case sem::IntrinsicType::kNormalize:
    case sem::IntrinsicType::kPow:
    case sem::IntrinsicType::kReflect:
    case sem::IntrinsicType::kRound:
    case sem::IntrinsicType::kSign:
    case sem::IntrinsicType::kSin:
    case sem::IntrinsicType::kSinh:
    case sem::IntrinsicType::kSqrt:
    case sem::IntrinsicType::kStep:
    case sem::IntrinsicType::kTan:
    case sem::IntrinsicType::kTanh:
    case sem::IntrinsicType::kTranspose:
    case sem::IntrinsicType::kTrunc:
      return intrinsic->str();
    case sem::IntrinsicType::kCountOneBits:
      return "countbits";
    case sem::IntrinsicType::kDpdx:
      return "ddx";
    case sem::IntrinsicType::kDpdxCoarse:
      return "ddx_coarse";
    case sem::IntrinsicType::kDpdxFine:
      return "ddx_fine";
    case sem::IntrinsicType::kDpdy:
      return "ddy";
    case sem::IntrinsicType::kDpdyCoarse:
      return "ddy_coarse";
    case sem::IntrinsicType::kDpdyFine:
      return "ddy_fine";
    case sem::IntrinsicType::kFaceForward:
      return "faceforward";
    case sem::IntrinsicType::kFract:
      return "frac";
    case sem::IntrinsicType::kFma:
      return "mad";
    case sem::IntrinsicType::kFwidth:
    case sem::IntrinsicType::kFwidthCoarse:
    case sem::IntrinsicType::kFwidthFine:
      return "fwidth";
    case sem::IntrinsicType::kInverseSqrt:
      return "rsqrt";
    case sem::IntrinsicType::kIsFinite:
      return "isfinite";
    case sem::IntrinsicType::kIsInf:
      return "isinf";
    case sem::IntrinsicType::kIsNan:
      return "isnan";
    case sem::IntrinsicType::kMix:
      return "lerp";
    case sem::IntrinsicType::kReverseBits:
      return "reversebits";
    case sem::IntrinsicType::kSmoothStep:
      return "smoothstep";
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Unknown builtin method: " + std::string(intrinsic->str()));
  }

  return "";
}

bool GeneratorImpl::EmitCase(ast::CaseStatement* stmt) {
  if (stmt->IsDefault()) {
    line() << "default: {";
  } else {
    for (auto* selector : stmt->selectors()) {
      auto out = line();
      out << "case ";
      if (!EmitLiteral(out, selector)) {
        return false;
      }
      out << ":";
      if (selector == stmt->selectors().back()) {
        out << " {";
      }
    }
  }

  {
    ScopedIndent si(this);
    if (!EmitStatements(stmt->body()->statements())) {
      return false;
    }
    if (!last_is_break_or_fallthrough(stmt->body())) {
      line() << "break;";
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitConstructor(std::ostream& out,
                                    ast::ConstructorExpression* expr) {
  if (auto* scalar = expr->As<ast::ScalarConstructorExpression>()) {
    return EmitScalarConstructor(out, scalar);
  }
  return EmitTypeConstructor(out, expr->As<ast::TypeConstructorExpression>());
}

bool GeneratorImpl::EmitScalarConstructor(
    std::ostream& out,
    ast::ScalarConstructorExpression* expr) {
  return EmitLiteral(out, expr->literal());
}

bool GeneratorImpl::EmitTypeConstructor(std::ostream& out,
                                        ast::TypeConstructorExpression* expr) {
  auto* type = TypeOf(expr)->UnwrapRef();

  // If the type constructor is empty then we need to construct with the zero
  // value for all components.
  if (expr->values().empty()) {
    return EmitZeroValue(out, type);
  }

  bool brackets = type->IsAnyOf<sem::Array, sem::Struct>();

  // For single-value vector initializers, swizzle the scalar to the right
  // vector dimension using .x
  const bool is_single_value_vector_init =
      type->is_scalar_vector() && expr->values().size() == 1 &&
      TypeOf(expr->values()[0])->is_scalar();

  auto it = structure_builders_.find(As<sem::Struct>(type));
  if (it != structure_builders_.end()) {
    out << it->second << "(";
    brackets = false;
  } else if (brackets) {
    out << "{";
  } else {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                  "")) {
      return false;
    }
    out << "(";
  }

  if (is_single_value_vector_init) {
    out << "(";
  }

  bool first = true;
  for (auto* e : expr->values()) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(out, e)) {
      return false;
    }
  }

  if (is_single_value_vector_init) {
    out << ")." << std::string(type->As<sem::Vector>()->size(), 'x');
  }

  out << (brackets ? "}" : ")");
  return true;
}

bool GeneratorImpl::EmitContinue(ast::ContinueStatement*) {
  if (!emit_continuing_()) {
    return false;
  }
  line() << "continue;";
  return true;
}

bool GeneratorImpl::EmitDiscard(ast::DiscardStatement*) {
  // TODO(dsinclair): Verify this is correct when the discard semantics are
  // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
  line() << "discard;";
  return true;
}

bool GeneratorImpl::EmitExpression(std::ostream& out, ast::Expression* expr) {
  if (auto* a = expr->As<ast::ArrayAccessorExpression>()) {
    return EmitArrayAccessor(out, a);
  }
  if (auto* b = expr->As<ast::BinaryExpression>()) {
    return EmitBinary(out, b);
  }
  if (auto* b = expr->As<ast::BitcastExpression>()) {
    return EmitBitcast(out, b);
  }
  if (auto* c = expr->As<ast::CallExpression>()) {
    return EmitCall(out, c);
  }
  if (auto* c = expr->As<ast::ConstructorExpression>()) {
    return EmitConstructor(out, c);
  }
  if (auto* i = expr->As<ast::IdentifierExpression>()) {
    return EmitIdentifier(out, i);
  }
  if (auto* m = expr->As<ast::MemberAccessorExpression>()) {
    return EmitMemberAccessor(out, m);
  }
  if (auto* u = expr->As<ast::UnaryOpExpression>()) {
    return EmitUnaryOp(out, u);
  }

  diagnostics_.add_error(diag::System::Writer,
                         "unknown expression type: " + builder_.str(expr));
  return false;
}

bool GeneratorImpl::EmitIdentifier(std::ostream& out,
                                   ast::IdentifierExpression* expr) {
  out << builder_.Symbols().NameFor(expr->symbol());
  return true;
}

bool GeneratorImpl::EmitIf(ast::IfStatement* stmt) {
  {
    auto out = line();
    out << "if (";
    if (!EmitExpression(out, stmt->condition())) {
      return false;
    }
    out << ") {";
  }

  if (!EmitStatementsWithIndent(stmt->body()->statements())) {
    return false;
  }

  for (auto* e : stmt->else_statements()) {
    if (e->HasCondition()) {
      line() << "} else {";
      increment_indent();

      {
        auto out = line();
        out << "if (";
        if (!EmitExpression(out, e->condition())) {
          return false;
        }
        out << ") {";
      }
    } else {
      line() << "} else {";
    }

    if (!EmitStatementsWithIndent(e->body()->statements())) {
      return false;
    }
  }

  line() << "}";

  for (auto* e : stmt->else_statements()) {
    if (e->HasCondition()) {
      decrement_indent();
      line() << "}";
    }
  }
  return true;
}

bool GeneratorImpl::EmitFunction(ast::Function* func) {
  auto* sem = builder_.Sem().Get(func);

  if (ast::HasDecoration<ast::InternalDecoration>(func->decorations())) {
    // An internal function. Do not emit.
    return true;
  }

  {
    auto out = line();
    auto name = builder_.Symbols().NameFor(func->symbol());
    // If the function returns an array, then we need to declare a typedef for
    // this.
    if (sem->ReturnType()->Is<sem::Array>()) {
      auto typedef_name = UniqueIdentifier(name + "_ret");
      auto pre = line();
      pre << "typedef ";
      if (!EmitTypeAndName(pre, sem->ReturnType(), ast::StorageClass::kNone,
                           ast::Access::kReadWrite, typedef_name)) {
        return false;
      }
      pre << ";";
      out << typedef_name;
    } else {
      if (!EmitType(out, sem->ReturnType(), ast::StorageClass::kNone,
                    ast::Access::kReadWrite, "")) {
        return false;
      }
    }

    out << " " << name << "(";

    bool first = true;

    for (auto* v : sem->Parameters()) {
      if (!first) {
        out << ", ";
      }
      first = false;

      auto const* type = v->Type();

      if (auto* ptr = type->As<sem::Pointer>()) {
        // Transform pointer parameters in to `inout` parameters.
        // The WGSL spec is highly restrictive in what can be passed in pointer
        // parameters, which allows for this transformation. See:
        // https://gpuweb.github.io/gpuweb/wgsl/#function-restriction
        out << "inout ";
        type = ptr->StoreType();
      }

      // Note: WGSL only allows for StorageClass::kNone on parameters, however
      // the sanitizer transforms generates load / store functions for storage
      // or uniform buffers. These functions have a buffer parameter with
      // StorageClass::kStorage or StorageClass::kUniform. This is required to
      // correctly translate the parameter to a [RW]ByteAddressBuffer for
      // storage buffers and a uint4[N] for uniform buffers.
      if (!EmitTypeAndName(
              out, type, v->StorageClass(), v->Access(),
              builder_.Symbols().NameFor(v->Declaration()->symbol()))) {
        return false;
      }
    }
    out << ") {";
  }

  if (!EmitStatementsWithIndent(func->body()->statements())) {
    return false;
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitGlobalVariable(ast::Variable* global) {
  if (global->is_const()) {
    return EmitProgramConstVariable(global);
  }

  auto* sem = builder_.Sem().Get(global);
  switch (sem->StorageClass()) {
    case ast::StorageClass::kUniform:
      return EmitUniformVariable(sem);
    case ast::StorageClass::kStorage:
      return EmitStorageVariable(sem);
    case ast::StorageClass::kUniformConstant:
      return EmitHandleVariable(sem);
    case ast::StorageClass::kPrivate:
      return EmitPrivateVariable(sem);
    case ast::StorageClass::kWorkgroup:
      return EmitWorkgroupVariable(sem);
    default:
      break;
  }

  TINT_ICE(Writer, diagnostics_)
      << "unhandled storage class " << sem->StorageClass();
  return false;
}

bool GeneratorImpl::EmitUniformVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto binding_point = decl->binding_point();
  auto* type = var->Type()->UnwrapRef();

  auto* str = type->As<sem::Struct>();
  if (!str) {
    // https://www.w3.org/TR/WGSL/#module-scope-variables
    TINT_ICE(Writer, diagnostics_)
        << "variables with uniform storage must be structure";
  }

  auto name = builder_.Symbols().NameFor(decl->symbol());
  line() << "cbuffer cbuffer_" << name << RegisterAndSpace('b', binding_point)
         << " {";

  {
    ScopedIndent si(this);
    auto out = line();
    if (!EmitTypeAndName(out, type, ast::StorageClass::kUniform, var->Access(),
                         name)) {
      return false;
    }
    out << ";";
  }

  line() << "};";

  return true;
}

bool GeneratorImpl::EmitStorageVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* type = var->Type()->UnwrapRef();
  auto out = line();
  if (!EmitTypeAndName(out, type, ast::StorageClass::kStorage, var->Access(),
                       builder_.Symbols().NameFor(decl->symbol()))) {
    return false;
  }

  out << RegisterAndSpace(var->Access() == ast::Access::kRead ? 't' : 'u',
                          decl->binding_point())
      << ";";

  return true;
}

bool GeneratorImpl::EmitHandleVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* unwrapped_type = var->Type()->UnwrapRef();
  auto out = line();

  auto name = builder_.Symbols().NameFor(decl->symbol());
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  const char* register_space = nullptr;

  if (unwrapped_type->Is<sem::Texture>()) {
    register_space = "t";
    if (auto* storage_tex = unwrapped_type->As<sem::StorageTexture>()) {
      if (storage_tex->access() != ast::Access::kRead) {
        register_space = "u";
      }
    }
  } else if (unwrapped_type->Is<sem::Sampler>()) {
    register_space = "s";
  }

  if (register_space) {
    auto bp = decl->binding_point();
    out << " : register(" << register_space << bp.binding->value() << ", space"
        << bp.group->value() << ")";
  }

  out << ";";
  return true;
}

bool GeneratorImpl::EmitPrivateVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto out = line();

  out << "static ";

  auto name = builder_.Symbols().NameFor(decl->symbol());
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  out << " = ";
  if (auto* constructor = decl->constructor()) {
    if (!EmitExpression(out, constructor)) {
      return false;
    }
  } else {
    if (!EmitZeroValue(out, var->Type()->UnwrapRef())) {
      return false;
    }
  }

  out << ";";
  return true;
}

bool GeneratorImpl::EmitWorkgroupVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto out = line();

  out << "groupshared ";

  auto name = builder_.Symbols().NameFor(decl->symbol());
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  if (auto* constructor = decl->constructor()) {
    out << " = ";
    if (!EmitExpression(out, constructor)) {
      return false;
    }
  }

  out << ";";
  return true;
}

std::string GeneratorImpl::builtin_to_attribute(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return "SV_Position";
    case ast::Builtin::kVertexIndex:
      return "SV_VertexID";
    case ast::Builtin::kInstanceIndex:
      return "SV_InstanceID";
    case ast::Builtin::kFrontFacing:
      return "SV_IsFrontFace";
    case ast::Builtin::kFragDepth:
      return "SV_Depth";
    case ast::Builtin::kLocalInvocationId:
      return "SV_GroupThreadID";
    case ast::Builtin::kLocalInvocationIndex:
      return "SV_GroupIndex";
    case ast::Builtin::kGlobalInvocationId:
      return "SV_DispatchThreadID";
    case ast::Builtin::kWorkgroupId:
      return "SV_GroupID";
    case ast::Builtin::kSampleIndex:
      return "SV_SampleIndex";
    case ast::Builtin::kSampleMask:
      return "SV_Coverage";
    default:
      break;
  }
  return "";
}

std::string GeneratorImpl::interpolation_to_modifiers(
    ast::InterpolationType type,
    ast::InterpolationSampling sampling) const {
  std::string modifiers;
  switch (type) {
    case ast::InterpolationType::kPerspective:
      modifiers += "linear ";
      break;
    case ast::InterpolationType::kLinear:
      modifiers += "noperspective ";
      break;
    case ast::InterpolationType::kFlat:
      modifiers += "nointerpolation ";
      break;
  }
  switch (sampling) {
    case ast::InterpolationSampling::kCentroid:
      modifiers += "centroid ";
      break;
    case ast::InterpolationSampling::kSample:
      modifiers += "sample ";
      break;
    case ast::InterpolationSampling::kCenter:
    case ast::InterpolationSampling::kNone:
      break;
  }
  return modifiers;
}

bool GeneratorImpl::EmitEntryPointFunction(ast::Function* func) {
  auto* func_sem = builder_.Sem().Get(func);

  {
    auto out = line();
    if (func->pipeline_stage() == ast::PipelineStage::kCompute) {
      // Emit the workgroup_size attribute.
      auto wgsize = func_sem->workgroup_size();
      out << "[numthreads(";
      for (int i = 0; i < 3; i++) {
        if (i > 0) {
          out << ", ";
        }

        if (wgsize[i].overridable_const) {
          auto* sem_const = builder_.Sem().Get(wgsize[i].overridable_const);
          if (!sem_const->IsPipelineConstant()) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "expected a pipeline-overridable constant";
          }
          out << kSpecConstantPrefix << sem_const->ConstantId();
        } else {
          out << std::to_string(wgsize[i].value);
        }
      }
      out << ")]" << std::endl;
    }

    out << func->return_type()->FriendlyName(builder_.Symbols());

    out << " " << builder_.Symbols().NameFor(func->symbol()) << "(";

    bool first = true;

    // Emit entry point parameters.
    for (auto* var : func->params()) {
      auto* sem = builder_.Sem().Get(var);
      auto* type = sem->Type();
      if (!type->Is<sem::Struct>()) {
        // ICE likely indicates that the CanonicalizeEntryPointIO transform was
        // not run, or a builtin parameter was added after it was run.
        TINT_ICE(Writer, diagnostics_)
            << "Unsupported non-struct entry point parameter";
      }

      if (!first) {
        out << ", ";
      }
      first = false;

      if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                           builder_.Symbols().NameFor(var->symbol()))) {
        return false;
      }
    }

    out << ") {";
  }

  {
    ScopedIndent si(this);

    if (!EmitStatements(func->body()->statements())) {
      return false;
    }

    if (!Is<ast::ReturnStatement>(func->get_last_statement())) {
      ast::ReturnStatement ret(ProgramID(), Source{});
      if (!EmitStatement(&ret)) {
        return false;
      }
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitLiteral(std::ostream& out, ast::Literal* lit) {
  if (auto* l = lit->As<ast::BoolLiteral>()) {
    out << (l->IsTrue() ? "true" : "false");
  } else if (auto* fl = lit->As<ast::FloatLiteral>()) {
    out << FloatToString(fl->value()) << "f";
  } else if (auto* sl = lit->As<ast::SintLiteral>()) {
    out << sl->value();
  } else if (auto* ul = lit->As<ast::UintLiteral>()) {
    out << ul->value() << "u";
  } else {
    diagnostics_.add_error(diag::System::Writer, "unknown literal type");
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitZeroValue(std::ostream& out, const sem::Type* type) {
  if (type->Is<sem::Bool>()) {
    out << "false";
  } else if (type->Is<sem::F32>()) {
    out << "0.0f";
  } else if (type->Is<sem::I32>()) {
    out << "0";
  } else if (type->Is<sem::U32>()) {
    out << "0u";
  } else if (auto* vec = type->As<sem::Vector>()) {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                  "")) {
      return false;
    }
    ScopedParen sp(out);
    for (uint32_t i = 0; i < vec->size(); i++) {
      if (i != 0) {
        out << ", ";
      }
      if (!EmitZeroValue(out, vec->type())) {
        return false;
      }
    }
  } else if (auto* mat = type->As<sem::Matrix>()) {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                  "")) {
      return false;
    }
    ScopedParen sp(out);
    for (uint32_t i = 0; i < (mat->rows() * mat->columns()); i++) {
      if (i != 0) {
        out << ", ";
      }
      if (!EmitZeroValue(out, mat->type())) {
        return false;
      }
    }
  } else if (type->IsAnyOf<sem::Struct, sem::Array>()) {
    out << "(";
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kUndefined,
                  "")) {
      return false;
    }
    out << ")0";
  } else {
    diagnostics_.add_error(
        diag::System::Writer,
        "Invalid type for zero emission: " + type->type_name());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitLoop(ast::LoopStatement* stmt) {
  auto emit_continuing = [this, stmt]() {
    if (stmt->has_continuing()) {
      if (!EmitBlock(stmt->continuing())) {
        return false;
      }
    }
    return true;
  };

  TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
  line() << "while (true) {";
  {
    ScopedIndent si(this);
    if (!EmitStatements(stmt->body()->statements())) {
      return false;
    }
    if (!emit_continuing()) {
      return false;
    }
  }
  line() << "}";

  return true;
}

bool GeneratorImpl::EmitForLoop(ast::ForLoopStatement* stmt) {
  TextBuffer init_buf;
  if (auto* init = stmt->initializer()) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
    if (!EmitStatement(init)) {
      return false;
    }
  }
  bool multi_stmt_init = init_buf.lines.size() > 1;
  // For-loop has multi-statement initializer.
  // This cannot be emitted with a regular for loop, so instead nest the loop in
  // a new block scope prefixed with these initializer statements.
  if (multi_stmt_init) {
    line() << "{";
    increment_indent();
    current_buffer_->Append(init_buf);
    init_buf.lines.clear();  // Don't emit the initializer again in the 'for'
  }

  TextBuffer cond_pre;
  std::stringstream cond_buf;
  if (auto* cond = stmt->condition()) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
    if (!EmitExpression(cond_buf, cond)) {
      return false;
    }
  }

  TextBuffer cont_buf;
  if (auto* cont = stmt->continuing()) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
    if (!EmitStatement(cont)) {
      return false;
    }
  }

  if (cond_pre.lines.size() > 0 || cont_buf.lines.size() > 1) {
    // For-loop has multi-statement conditional and / or continuing.
    // This cannot be emitted with a regular for loop, so instead generate a
    // `while(true)` loop.
    auto emit_continuing = [&]() {
      current_buffer_->Append(cont_buf);
      return true;
    };

    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
    line() << "while (true) {";
    {
      ScopedIndent si(this);

      if (stmt->condition()) {
        current_buffer_->Append(cond_pre);
        line() << "if (!(" << cond_buf.str() << ")) { break; }";
      }

      if (!EmitStatements(stmt->body()->statements())) {
        return false;
      }

      if (!emit_continuing()) {
        return false;
      }
    }
    line() << "}";
  } else {
    // For-loop can be generated.
    {
      auto out = line();
      out << "for";
      {
        ScopedParen sp(out);

        if (!init_buf.lines.empty()) {
          out << init_buf.lines[0].content << " ";
        } else {
          out << "; ";
        }

        out << cond_buf.str() << "; ";

        if (!cont_buf.lines.empty()) {
          out << TrimSuffix(cont_buf.lines[0].content, ";");
        }
      }
      out << " {";
    }
    {
      auto emit_continuing = [] { return true; };
      TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
      if (!EmitStatementsWithIndent(stmt->body()->statements())) {
        return false;
      }
    }

    line() << "}";
  }

  if (multi_stmt_init) {
    decrement_indent();
    line() << "}";
  }

  return true;
}

bool GeneratorImpl::EmitMemberAccessor(std::ostream& out,
                                       ast::MemberAccessorExpression* expr) {
  if (!EmitExpression(out, expr->structure())) {
    return false;
  }
  out << ".";

  // Swizzles output the name directly
  if (builder_.Sem().Get(expr)->Is<sem::Swizzle>()) {
    out << builder_.Symbols().NameFor(expr->member()->symbol());
  } else if (!EmitExpression(out, expr->member())) {
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitReturn(ast::ReturnStatement* stmt) {
  if (stmt->has_value()) {
    auto out = line();
    out << "return ";
    if (!EmitExpression(out, stmt->value())) {
      return false;
    }
    out << ";";
  } else {
    line() << "return;";
  }
  return true;
}

bool GeneratorImpl::EmitStatement(ast::Statement* stmt) {
  if (auto* a = stmt->As<ast::AssignmentStatement>()) {
    return EmitAssign(a);
  }
  if (auto* b = stmt->As<ast::BlockStatement>()) {
    return EmitBlock(b);
  }
  if (auto* b = stmt->As<ast::BreakStatement>()) {
    return EmitBreak(b);
  }
  if (auto* c = stmt->As<ast::CallStatement>()) {
    auto out = line();
    if (!TypeOf(c->expr())->Is<sem::Void>()) {
      out << "(void) ";
    }
    if (!EmitCall(out, c->expr())) {
      return false;
    }
    out << ";";
    return true;
  }
  if (auto* c = stmt->As<ast::ContinueStatement>()) {
    return EmitContinue(c);
  }
  if (auto* d = stmt->As<ast::DiscardStatement>()) {
    return EmitDiscard(d);
  }
  if (stmt->As<ast::FallthroughStatement>()) {
    line() << "/* fallthrough */";
    return true;
  }
  if (auto* i = stmt->As<ast::IfStatement>()) {
    return EmitIf(i);
  }
  if (auto* l = stmt->As<ast::LoopStatement>()) {
    return EmitLoop(l);
  }
  if (auto* l = stmt->As<ast::ForLoopStatement>()) {
    return EmitForLoop(l);
  }
  if (auto* r = stmt->As<ast::ReturnStatement>()) {
    return EmitReturn(r);
  }
  if (auto* s = stmt->As<ast::SwitchStatement>()) {
    return EmitSwitch(s);
  }
  if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
    return EmitVariable(v->variable());
  }

  diagnostics_.add_error(diag::System::Writer,
                         "unknown statement type: " + builder_.str(stmt));
  return false;
}

bool GeneratorImpl::EmitSwitch(ast::SwitchStatement* stmt) {
  {  // switch(expr) {
    auto out = line();
    out << "switch(";
    if (!EmitExpression(out, stmt->condition())) {
      return false;
    }
    out << ") {";
  }

  {
    ScopedIndent si(this);
    for (auto* s : stmt->body()) {
      if (!EmitCase(s)) {
        return false;
      }
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitType(std::ostream& out,
                             const sem::Type* type,
                             ast::StorageClass storage_class,
                             ast::Access access,
                             const std::string& name,
                             bool* name_printed /* = nullptr */) {
  switch (storage_class) {
    case ast::StorageClass::kStorage:
      if (access != ast::Access::kRead) {
        out << "RW";
      }
      out << "ByteAddressBuffer";
      return true;
    case ast::StorageClass::kUniform: {
      auto* str = type->As<sem::Struct>();
      if (!str) {
        // https://www.w3.org/TR/WGSL/#module-scope-variables
        TINT_ICE(Writer, diagnostics_)
            << "variables with uniform storage must be structure";
      }
      auto array_length = (str->Size() + 15) / 16;
      out << "uint4 " << name << "[" << array_length << "]";
      if (name_printed) {
        *name_printed = true;
      }
      return true;
    }
    default:
      break;
  }

  if (auto* ary = type->As<sem::Array>()) {
    const sem::Type* base_type = ary;
    std::vector<uint32_t> sizes;
    while (auto* arr = base_type->As<sem::Array>()) {
      if (arr->IsRuntimeSized()) {
        TINT_ICE(Writer, diagnostics_)
            << "Runtime arrays may only exist in storage buffers, which should "
               "have been transformed into a ByteAddressBuffer";
        return false;
      }
      sizes.push_back(arr->Count());
      base_type = arr->ElemType();
    }
    if (!EmitType(out, base_type, storage_class, access, "")) {
      return false;
    }
    if (!name.empty()) {
      out << " " << name;
      if (name_printed) {
        *name_printed = true;
      }
    }
    for (uint32_t size : sizes) {
      out << "[" << size << "]";
    }
  } else if (type->Is<sem::Bool>()) {
    out << "bool";
  } else if (type->Is<sem::F32>()) {
    out << "float";
  } else if (type->Is<sem::I32>()) {
    out << "int";
  } else if (auto* mat = type->As<sem::Matrix>()) {
    if (!EmitType(out, mat->type(), storage_class, access, "")) {
      return false;
    }
    // Note: HLSL's matrices are declared as <type>NxM, where N is the number of
    // rows and M is the number of columns. Despite HLSL's matrices being
    // column-major by default, the index operator and constructors actually
    // operate on row-vectors, where as WGSL operates on column vectors.
    // To simplify everything we use the transpose of the matrices.
    // See:
    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
    out << mat->columns() << "x" << mat->rows();
  } else if (type->Is<sem::Pointer>()) {
    TINT_ICE(Writer, diagnostics_)
        << "Attempting to emit pointer type. These should have been removed "
           "with the InlinePointerLets transform";
    return false;
  } else if (auto* sampler = type->As<sem::Sampler>()) {
    out << "Sampler";
    if (sampler->IsComparison()) {
      out << "Comparison";
    }
    out << "State";
  } else if (auto* str = type->As<sem::Struct>()) {
    out << builder_.Symbols().NameFor(str->Declaration()->name());
  } else if (auto* tex = type->As<sem::Texture>()) {
    auto* storage = tex->As<sem::StorageTexture>();
    auto* multism = tex->As<sem::MultisampledTexture>();
    auto* sampled = tex->As<sem::SampledTexture>();

    if (storage && storage->access() != ast::Access::kRead) {
      out << "RW";
    }
    out << "Texture";

    switch (tex->dim()) {
      case ast::TextureDimension::k1d:
        out << "1D";
        break;
      case ast::TextureDimension::k2d:
        out << (multism ? "2DMS" : "2D");
        break;
      case ast::TextureDimension::k2dArray:
        out << (multism ? "2DMSArray" : "2DArray");
        break;
      case ast::TextureDimension::k3d:
        out << "3D";
        break;
      case ast::TextureDimension::kCube:
        out << "Cube";
        break;
      case ast::TextureDimension::kCubeArray:
        out << "CubeArray";
        break;
      default:
        TINT_UNREACHABLE(Writer, diagnostics_)
            << "unexpected TextureDimension " << tex->dim();
        return false;
    }

    if (storage) {
      auto* component = image_format_to_rwtexture_type(storage->image_format());
      if (component == nullptr) {
        TINT_ICE(Writer, diagnostics_)
            << "Unsupported StorageTexture ImageFormat: "
            << static_cast<int>(storage->image_format());
        return false;
      }
      out << "<" << component << ">";
    } else if (sampled || multism) {
      auto* subtype = sampled ? sampled->type() : multism->type();
      out << "<";
      if (subtype->Is<sem::F32>()) {
        out << "float4";
      } else if (subtype->Is<sem::I32>()) {
        out << "int4";
      } else if (subtype->Is<sem::U32>()) {
        out << "uint4";
      } else {
        TINT_ICE(Writer, diagnostics_)
            << "Unsupported multisampled texture type";
        return false;
      }
      out << ">";
    }
  } else if (type->Is<sem::U32>()) {
    out << "uint";
  } else if (auto* vec = type->As<sem::Vector>()) {
    auto size = vec->size();
    if (vec->type()->Is<sem::F32>() && size >= 1 && size <= 4) {
      out << "float" << size;
    } else if (vec->type()->Is<sem::I32>() && size >= 1 && size <= 4) {
      out << "int" << size;
    } else if (vec->type()->Is<sem::U32>() && size >= 1 && size <= 4) {
      out << "uint" << size;
    } else if (vec->type()->Is<sem::Bool>() && size >= 1 && size <= 4) {
      out << "bool" << size;
    } else {
      out << "vector<";
      if (!EmitType(out, vec->type(), storage_class, access, "")) {
        return false;
      }
      out << ", " << size << ">";
    }
  } else if (auto* atomic = type->As<sem::Atomic>()) {
    if (!EmitType(out, atomic->Type(), storage_class, access, name)) {
      return false;
    }
  } else if (type->Is<sem::Void>()) {
    out << "void";
  } else {
    diagnostics_.add_error(diag::System::Writer, "unknown type in EmitType");
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitTypeAndName(std::ostream& out,
                                    const sem::Type* type,
                                    ast::StorageClass storage_class,
                                    ast::Access access,
                                    const std::string& name) {
  bool printed_name = false;
  if (!EmitType(out, type, storage_class, access, name, &printed_name)) {
    return false;
  }
  if (!name.empty() && !printed_name) {
    out << " " << name;
  }
  return true;
}

bool GeneratorImpl::EmitStructType(const sem::Struct* str) {
  auto storage_class_uses = str->StorageClassUsage();
  if (storage_class_uses.size() ==
      (storage_class_uses.count(ast::StorageClass::kStorage) +
       storage_class_uses.count(ast::StorageClass::kUniform))) {
    // The only use of the structure is as a storage buffer and / or uniform
    // buffer.
    // Structures used as storage buffer are read and written to via a
    // ByteAddressBuffer instead of true structure.
    // Structures used as uniform buffer are read from an array of vectors
    // instead of true structure.
    return true;
  }

  auto struct_name = builder_.Symbols().NameFor(str->Declaration()->name());
  line() << "struct " << struct_name << " {";
  {
    ScopedIndent si(this);
    for (auto* mem : str->Members()) {
      auto name = builder_.Symbols().NameFor(mem->Declaration()->symbol());

      auto* ty = mem->Type();

      auto out = line();

      std::string pre, post;

      for (auto* deco : mem->Declaration()->decorations()) {
        if (auto* location = deco->As<ast::LocationDecoration>()) {
          auto& pipeline_stage_uses = str->PipelineStageUses();
          if (pipeline_stage_uses.size() != 1) {
            TINT_ICE(Writer, diagnostics_)
                << "invalid entry point IO struct uses";
          }

          if (pipeline_stage_uses.count(
                  sem::PipelineStageUsage::kVertexInput)) {
            post += " : TEXCOORD" + std::to_string(location->value());
          } else if (pipeline_stage_uses.count(
                         sem::PipelineStageUsage::kVertexOutput)) {
            post += " : TEXCOORD" + std::to_string(location->value());
          } else if (pipeline_stage_uses.count(
                         sem::PipelineStageUsage::kFragmentInput)) {
            post += " : TEXCOORD" + std::to_string(location->value());
          } else if (pipeline_stage_uses.count(
                         sem::PipelineStageUsage::kFragmentOutput)) {
            post += " : SV_Target" + std::to_string(location->value());
          } else {
            TINT_ICE(Writer, diagnostics_)
                << "invalid use of location decoration";
          }
        } else if (auto* builtin = deco->As<ast::BuiltinDecoration>()) {
          auto attr = builtin_to_attribute(builtin->value());
          if (attr.empty()) {
            diagnostics_.add_error(diag::System::Writer, "unsupported builtin");
            return false;
          }
          post += " : " + attr;
        } else if (auto* interpolate = deco->As<ast::InterpolateDecoration>()) {
          auto mod = interpolation_to_modifiers(interpolate->type(),
                                                interpolate->sampling());
          if (mod.empty()) {
            diagnostics_.add_error(diag::System::Writer,
                                   "unsupported interpolation");
            return false;
          }
          pre += mod;
        }
      }

      out << pre;
      if (!EmitTypeAndName(out, ty, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, name)) {
        return false;
      }
      out << post << ";";
    }
  }

  line() << "};";

  return true;
}

bool GeneratorImpl::EmitUnaryOp(std::ostream& out,
                                ast::UnaryOpExpression* expr) {
  switch (expr->op()) {
    case ast::UnaryOp::kIndirection:
    case ast::UnaryOp::kAddressOf:
      return EmitExpression(out, expr->expr());
    case ast::UnaryOp::kComplement:
      out << "~";
      break;
    case ast::UnaryOp::kNot:
      out << "!";
      break;
    case ast::UnaryOp::kNegation:
      out << "-";
      break;
  }
  out << "(";

  if (!EmitExpression(out, expr->expr())) {
    return false;
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitVariable(ast::Variable* var) {
  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type()->UnwrapRef();

  // TODO(dsinclair): Handle variable decorations
  if (!var->decorations().empty()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Variable decorations are not handled yet");
    return false;
  }

  auto out = line();
  if (var->is_const()) {
    out << "const ";
  }
  if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                       builder_.Symbols().NameFor(var->symbol()))) {
    return false;
  }

  out << " = ";

  if (var->constructor()) {
    if (!EmitExpression(out, var->constructor())) {
      return false;
    }
  } else {
    if (!EmitZeroValue(out, type)) {
      return false;
    }
  }
  out << ";";

  return true;
}

bool GeneratorImpl::EmitProgramConstVariable(const ast::Variable* var) {
  for (auto* d : var->decorations()) {
    if (!d->Is<ast::OverrideDecoration>()) {
      diagnostics_.add_error(diag::System::Writer,
                             "Decorated const values not valid");
      return false;
    }
  }
  if (!var->is_const()) {
    diagnostics_.add_error(diag::System::Writer, "Expected a const value");
    return false;
  }

  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type();

  if (sem->IsPipelineConstant()) {
    auto const_id = sem->ConstantId();

    line() << "#ifndef " << kSpecConstantPrefix << const_id;

    if (var->constructor() != nullptr) {
      auto out = line();
      out << "#define " << kSpecConstantPrefix << const_id << " ";
      if (!EmitExpression(out, var->constructor())) {
        return false;
      }
    } else {
      line() << "#error spec constant required for constant id " << const_id;
    }
    line() << "#endif";
    {
      auto out = line();
      out << "static const ";
      if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                           builder_.Symbols().NameFor(var->symbol()))) {
        return false;
      }
      out << " = " << kSpecConstantPrefix << const_id << ";";
    }
  } else {
    auto out = line();
    out << "static const ";
    if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                         builder_.Symbols().NameFor(var->symbol()))) {
      return false;
    }
    out << " = ";
    if (!EmitExpression(out, var->constructor())) {
      return false;
    }
    out << ";";
  }

  return true;
}

std::string GeneratorImpl::get_buffer_name(ast::Expression* expr) {
  for (;;) {
    if (auto* ident = expr->As<ast::IdentifierExpression>()) {
      return builder_.Symbols().NameFor(ident->symbol());
    } else if (auto* member = expr->As<ast::MemberAccessorExpression>()) {
      expr = member->structure();
    } else if (auto* array = expr->As<ast::ArrayAccessorExpression>()) {
      expr = array->array();
    } else {
      break;
    }
  }
  return "";
}

}  // namespace hlsl
}  // namespace writer
}  // namespace tint
