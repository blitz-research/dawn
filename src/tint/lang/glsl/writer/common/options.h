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

#ifndef SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_
#define SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_

#include <optional>
#include <string>
#include <unordered_map>

#include "src/tint/api/options/binding_remapper.h"
#include "src/tint/api/options/external_texture.h"
#include "src/tint/api/options/texture_builtins_from_uniform.h"
#include "src/tint/lang/core/access.h"
#include "src/tint/lang/glsl/writer/common/version.h"
#include "src/tint/lang/wgsl/sem/sampler_texture_pair.h"

namespace tint::glsl::writer {

using BindingMap = std::unordered_map<sem::SamplerTexturePair, std::string>;

/// Configuration options used for generating GLSL.
struct Options {
    /// Constructor
    Options();

    /// Destructor
    ~Options();

    /// Copy constructor
    Options(const Options&);

    /// Set to `true` to disable software robustness that prevents out-of-bounds accesses.
    bool disable_robustness = false;

    /// Set to `true` to disable workgroup memory zero initialization
    bool disable_workgroup_init = false;

    /// The GLSL version to emit
    Version version;

    /// A map of SamplerTexturePair to combined sampler names for the
    /// CombineSamplers transform
    BindingMap binding_map;

    /// The binding point to use for placeholder samplers.
    BindingPoint placeholder_binding_point;

    /// Options used in the bindings remapper
    BindingRemapperOptions binding_remapper_options = {};

    /// Options used in the binding mappings for external textures
    ExternalTextureOptions external_texture_options = {};

    /// Options used to map WGSL textureNumLevels/textureNumSamples builtins to internal uniform
    /// buffer values. If not specified, emits corresponding GLSL builtins
    /// textureQueryLevels/textureSamples directly.
    std::optional<TextureBuiltinsFromUniformOptions> texture_builtins_from_uniform = std::nullopt;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(disable_robustness,
                 disable_workgroup_init,
                 version,
                 binding_map,
                 placeholder_binding_point,
                 binding_remapper_options,
                 external_texture_options,
                 texture_builtins_from_uniform);
};

}  // namespace tint::glsl::writer

#endif  // SRC_TINT_LANG_GLSL_WRITER_COMMON_OPTIONS_H_
