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
//   test/intrinsics/intrinsics.wgsl.tmpl
// and the intrinsic defintion file:
//   src/intrinsics.def
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

[[group(1), binding(0)]] var arg_0: sampler;
fn ignore_5016e5() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_5016e5();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_5016e5();
}

[[stage(compute)]]
fn compute_main() {
  ignore_5016e5();
}
