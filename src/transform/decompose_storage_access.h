// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TRANSFORM_DECOMPOSE_STORAGE_ACCESS_H_
#define SRC_TRANSFORM_DECOMPOSE_STORAGE_ACCESS_H_

#include <string>

#include "src/ast/internal_decoration.h"
#include "src/transform/transform.h"

namespace tint {

// Forward declarations
class CloneContext;

namespace transform {

/// DecomposeStorageAccess is a transform used to replace storage buffer
/// accesses with a combination of load, store or atomic functions on primitive
/// types.
class DecomposeStorageAccess : public Transform {
 public:
  /// Intrinsic is an InternalDecoration that's used to decorate a stub function
  /// so that the HLSL transforms this into calls to
  /// `[RW]ByteAddressBuffer.Load[N]()` or `[RW]ByteAddressBuffer.Store[N]()`,
  /// with a possible cast.
  class Intrinsic : public Castable<Intrinsic, ast::InternalDecoration> {
   public:
    /// Intrinsic op
    enum class Op {
      kLoad,
      kStore,
      kAtomicLoad,
      kAtomicStore,
      kAtomicAdd,
      kAtomicMax,
      kAtomicMin,
      kAtomicAnd,
      kAtomicOr,
      kAtomicXor,
      kAtomicExchange,
      kAtomicCompareExchangeWeak,
    };

    /// Intrinsic data type
    enum class DataType {
      kU32,
      kF32,
      kI32,
      kVec2U32,
      kVec2F32,
      kVec2I32,
      kVec3U32,
      kVec3F32,
      kVec3I32,
      kVec4U32,
      kVec4F32,
      kVec4I32,
    };

    /// Constructor
    /// @param program_id the identifier of the program that owns this node
    /// @param o the op of the intrinsic
    /// @param ty the data type of the intrinsic
    Intrinsic(ProgramID program_id, Op o, DataType ty);
    /// Destructor
    ~Intrinsic() override;

    /// @return a short description of the internal decoration which will be
    /// displayed as `[[internal(<name>)]]`
    std::string InternalName() const override;

    /// Performs a deep clone of this object using the CloneContext `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned object
    Intrinsic* Clone(CloneContext* ctx) const override;

    /// The op of the intrinsic
    Op const op;

    /// The type of the intrinsic
    DataType const type;
  };

  /// Constructor
  DecomposeStorageAccess();
  /// Destructor
  ~DecomposeStorageAccess() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;

  struct State;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_DECOMPOSE_STORAGE_ACCESS_H_
