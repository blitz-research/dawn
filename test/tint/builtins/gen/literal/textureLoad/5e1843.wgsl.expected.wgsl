@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba16uint, read_write>;

fn textureLoad_5e1843() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<u32>(1u), 1u);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_5e1843();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_5e1843();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_5e1843();
}
