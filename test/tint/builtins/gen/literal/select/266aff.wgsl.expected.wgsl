fn select_266aff() {
  var res : vec2<f32> = select(vec2<f32>(1.0f), vec2<f32>(1.0f), vec2<bool>(true));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_266aff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_266aff();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_266aff();
}
