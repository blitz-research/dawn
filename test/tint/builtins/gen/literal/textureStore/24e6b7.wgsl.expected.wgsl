@group(1) @binding(0) var arg_0 : texture_storage_3d<r32sint, read_write>;

fn textureStore_24e6b7() {
  textureStore(arg_0, vec3<i32>(1i), vec4<i32>(1i));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_24e6b7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_24e6b7();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_24e6b7();
}
