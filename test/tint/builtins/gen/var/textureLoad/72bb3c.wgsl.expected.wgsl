@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8snorm, read>;

fn textureLoad_72bb3c() {
  var arg_1 = vec2<u32>(1u);
  var arg_2 = 1i;
  var res : vec4<f32> = textureLoad(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_72bb3c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_72bb3c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_72bb3c();
}
