@group(1) @binding(0) var arg_0 : texture_storage_3d<r32uint, read_write>;

fn textureStore_53a68b() {
  textureStore(arg_0, vec3<i32>(1i), vec4<u32>(1u));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_53a68b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_53a68b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_53a68b();
}
