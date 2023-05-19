@group(1) @binding(0) var arg_0 : texture_storage_1d<bgra8unorm, write>;

fn textureStore_c9d780() {
  textureStore(arg_0, 1u, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_c9d780();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_c9d780();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_c9d780();
}
