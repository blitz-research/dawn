@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rg32float, read_write>;

fn textureLoad_bdc67a() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<u32>(1u), 1i);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_bdc67a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_bdc67a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_bdc67a();
}
