enable chromium_experimental_read_write_storage_texture;

@group(1) @binding(0) var arg_0 : texture_storage_1d<r32float, read>;

fn textureLoad_c7cbed() {
  var res : vec4<f32> = textureLoad(arg_0, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_c7cbed();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_c7cbed();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_c7cbed();
}
