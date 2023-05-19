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
// File generated by tools/src/cmd/gen
// using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

@group(1) @binding(0) var arg_0: texture_cube_array<f32>;
@group(1) @binding(1) var arg_1: sampler;

// fn textureSampleGrad(texture: texture_cube_array<f32>, sampler: sampler, coords: vec3<f32>, array_index: u32, ddx: vec3<f32>, ddy: vec3<f32>) -> vec4<f32>
fn textureSampleGrad_bbb58f() {
  var arg_2 = vec3<f32>(1.f);
  var arg_3 = 1u;
  var arg_4 = vec3<f32>(1.f);
  var arg_5 = vec3<f32>(1.f);
  var res: vec4<f32> = textureSampleGrad(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleGrad_bbb58f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleGrad_bbb58f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleGrad_bbb58f();
}
