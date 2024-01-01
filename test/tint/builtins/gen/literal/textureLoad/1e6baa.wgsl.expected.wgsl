@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32float, read_write>;

fn textureLoad_1e6baa() {
  var res : vec4<f32> = textureLoad(arg_0, 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_1e6baa();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_1e6baa();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_1e6baa();
}
