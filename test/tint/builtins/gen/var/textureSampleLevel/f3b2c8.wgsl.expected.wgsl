@group(1) @binding(0) var arg_0 : texture_depth_2d;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSampleLevel_f3b2c8() {
  var arg_2 = vec2<f32>(1.0f);
  var arg_3 = 1u;
  const arg_4 = vec2<i32>(1i);
  var res : f32 = textureSampleLevel(arg_0, arg_1, arg_2, arg_3, arg_4);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureSampleLevel_f3b2c8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureSampleLevel_f3b2c8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureSampleLevel_f3b2c8();
}
