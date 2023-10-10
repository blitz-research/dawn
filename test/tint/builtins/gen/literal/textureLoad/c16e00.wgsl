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

@group(1) @binding(0) var arg_0: texture_depth_2d_array;

// fn textureLoad(texture: texture_depth_2d_array, coords: vec2<i32>, array_index: u32, level: i32) -> f32
fn textureLoad_c16e00() {
  var res: f32 = textureLoad(arg_0, vec2<i32>(1i), 1u, 1i);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_c16e00();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_c16e00();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_c16e00();
}
