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

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/intrinsic-gen
// using the template:
//   src/sem/intrinsic_type.h.tmpl
// and the intrinsic defintion file:
//   src/intrinsics.def
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_SEM_INTRINSIC_TYPE_H_
#define SRC_SEM_INTRINSIC_TYPE_H_

#include <string>

namespace tint {
namespace sem {

/// Enumerator of all intrinsic functions
enum class IntrinsicType {
  kNone = -1,
  kAbs,
  kAcos,
  kAll,
  kAny,
  kArrayLength,
  kAsin,
  kAtan,
  kAtan2,
  kCeil,
  kClamp,
  kCos,
  kCosh,
  kCountOneBits,
  kCross,
  kDeterminant,
  kDistance,
  kDot,
  kDpdx,
  kDpdxCoarse,
  kDpdxFine,
  kDpdy,
  kDpdyCoarse,
  kDpdyFine,
  kExp,
  kExp2,
  kFaceForward,
  kFloor,
  kFma,
  kFract,
  kFrexp,
  kFwidth,
  kFwidthCoarse,
  kFwidthFine,
  kIgnore,
  kInverseSqrt,
  kIsFinite,
  kIsInf,
  kIsNan,
  kIsNormal,
  kLdexp,
  kLength,
  kLog,
  kLog2,
  kMax,
  kMin,
  kMix,
  kModf,
  kNormalize,
  kPack2x16float,
  kPack2x16snorm,
  kPack2x16unorm,
  kPack4x8snorm,
  kPack4x8unorm,
  kPow,
  kReflect,
  kReverseBits,
  kRound,
  kSelect,
  kSign,
  kSin,
  kSinh,
  kSmoothStep,
  kSqrt,
  kStep,
  kStorageBarrier,
  kTan,
  kTanh,
  kTrunc,
  kUnpack2x16float,
  kUnpack2x16snorm,
  kUnpack2x16unorm,
  kUnpack4x8snorm,
  kUnpack4x8unorm,
  kWorkgroupBarrier,
  kTextureDimensions,
  kTextureNumLayers,
  kTextureNumLevels,
  kTextureNumSamples,
  kTextureSample,
  kTextureSampleBias,
  kTextureSampleCompare,
  kTextureSampleGrad,
  kTextureSampleLevel,
  kTextureStore,
  kTextureLoad,
};

/// Matches the IntrinsicType by name
/// @param name the intrinsic name to parse
/// @returns the parsed IntrinsicType, or IntrinsicType::kNone if `name` did not
/// match any intrinsic.
IntrinsicType ParseIntrinsicType(const std::string& name);

/// @returns the name of the intrinsic function type. The spelling, including
/// case, matches the name in the WGSL spec.
const char* str(IntrinsicType i);

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_INTRINSIC_TYPE_H_
