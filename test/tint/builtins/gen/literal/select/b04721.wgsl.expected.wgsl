fn select_b04721() {
  var res : vec3<u32> = select(vec3<u32>(1u), vec3<u32>(1u), true);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_b04721();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_b04721();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_b04721();
}
