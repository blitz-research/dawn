@group(1) @binding(0) var arg_0 : texture_storage_1d<bgra8unorm, read_write>;

fn textureLoad_d80ff3() {
  var res : vec4<f32> = textureLoad(arg_0, 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_d80ff3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_d80ff3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_d80ff3();
}
