@group(1) @binding(0) var arg_0 : texture_storage_3d<rgba8unorm, read_write>;

fn textureStore_885921() {
  textureStore(arg_0, vec3<u32>(1u), vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_885921();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_885921();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_885921();
}
