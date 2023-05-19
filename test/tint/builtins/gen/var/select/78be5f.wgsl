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
// File generated by tools/src/cmd/gen
// using the template:
//   test/tint/builtins/gen/gen.wgsl.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////


// fn select(vec<3, f32>, vec<3, f32>, bool) -> vec<3, f32>
fn select_78be5f() {
  var arg_0 = vec3<f32>(1.f);
  var arg_1 = vec3<f32>(1.f);
  var arg_2 = true;
  var res: vec3<f32> = select(arg_0, arg_1, arg_2);
  prevent_dce = res;
}
@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_78be5f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_78be5f();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_78be5f();
}
