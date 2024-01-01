@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8sint, read_write>;

fn textureLoad_fc47ff() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_fc47ff();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_fc47ff();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_fc47ff();
}
