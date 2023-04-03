enable f16;

fn asinh_ad8f8b() {
  var arg_0 = vec2<f16>(1.0h);
  var res : vec2<f16> = asinh(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  asinh_ad8f8b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  asinh_ad8f8b();
}

@compute @workgroup_size(1)
fn compute_main() {
  asinh_ad8f8b();
}
