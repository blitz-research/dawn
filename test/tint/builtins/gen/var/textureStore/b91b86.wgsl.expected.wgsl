enable chromium_internal_graphite;

@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r8unorm, write>;

fn textureStore_b91b86() {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = 1i;
  var arg_3 = vec4<f32>(1.0f);
  textureStore(arg_0, arg_1, arg_2, arg_3);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureStore_b91b86();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureStore_b91b86();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureStore_b91b86();
}
