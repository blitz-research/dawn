enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16float, read_write>;

fn textureStore_e72bdc() {
  textureStore(arg_0, vec2<i32>(1i), 1u, vec4<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_e72bdc();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_e72bdc();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_e72bdc();
}
