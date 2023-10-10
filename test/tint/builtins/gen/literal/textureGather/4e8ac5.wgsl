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

////////////////////////////////////////////////////////////////////////////////
// File generated by 'tools/src/cmd/gen' using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

@group(1) @binding(1) var arg_1: texture_2d_array<i32>;
@group(1) @binding(2) var arg_2: sampler;

// fn textureGather(@const component: i32, texture: texture_2d_array<i32>, sampler: sampler, coords: vec2<f32>, array_index: u32) -> vec4<i32>
fn textureGather_4e8ac5() {
  var res: vec4<i32> = textureGather(1i, arg_1, arg_2, vec2<f32>(1.f), 1u);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_4e8ac5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_4e8ac5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_4e8ac5();
}
