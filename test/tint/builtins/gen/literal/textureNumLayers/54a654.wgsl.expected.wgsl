@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16sint, read_write>;

fn textureNumLayers_54a654() {
  var res : u32 = textureNumLayers(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : u32;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureNumLayers_54a654();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureNumLayers_54a654();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureNumLayers_54a654();
}
