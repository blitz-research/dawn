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
// File generated by 'tools/src/cmd/gen' using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// To regenerate run: './tools/run gen'
//
//                       Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

@group(1) @binding(0) var arg_0: texture_1d<i32>;

// fn textureLoad(texture: texture_1d<i32>, coords: i32, level: i32) -> vec4<i32>
fn textureLoad_5a2f9d() {
  var arg_1 = 1i;
  var arg_2 = 1i;
  var res: vec4<i32> = textureLoad(arg_0, arg_1, arg_2);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_5a2f9d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_5a2f9d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_5a2f9d();
}
