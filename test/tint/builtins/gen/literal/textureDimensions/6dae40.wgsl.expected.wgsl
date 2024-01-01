@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32float, read>;

fn textureDimensions_6dae40() {
  var res : vec3<u32> = textureDimensions(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_6dae40();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_6dae40();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_6dae40();
}
