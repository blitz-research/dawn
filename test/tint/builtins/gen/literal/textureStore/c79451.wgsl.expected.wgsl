@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16sint, read_write>;

fn textureStore_c79451() {
  textureStore(arg_0, 1u, vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_c79451();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_c79451();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c79451();
}
