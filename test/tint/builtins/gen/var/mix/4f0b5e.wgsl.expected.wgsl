fn mix_4f0b5e() {
  var arg_0 = 1.0f;
  var arg_1 = 1.0f;
  var arg_2 = 1.0f;
  var res : f32 = mix(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_4f0b5e();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_4f0b5e();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_4f0b5e();
}
