enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_2d<r32uint, read_write>;

fn textureStore_4bf1fd() {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = vec4<u32>(1u);
  textureStore(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_4bf1fd();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_4bf1fd();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_4bf1fd();
}
