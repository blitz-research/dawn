enable f16;

fn mix_38cbbb() {
  var arg_0 = 1.0h;
  var arg_1 = 1.0h;
  var arg_2 = 1.0h;
  var res : f16 = mix(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : f16;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_38cbbb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_38cbbb();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_38cbbb();
}
