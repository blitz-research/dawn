@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba8snorm, read_write>;

fn textureDimensions_a7ae4c() {
  var res : u32 = textureDimensions(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_a7ae4c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_a7ae4c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_a7ae4c();
}
