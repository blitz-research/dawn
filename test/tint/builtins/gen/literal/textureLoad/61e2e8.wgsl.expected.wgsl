@group(1) @binding(0) var arg_0 : texture_storage_3d<rg32sint, read_write>;

fn textureLoad_61e2e8() {
  var res : vec4<i32> = textureLoad(arg_0, vec3<u32>(1u));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_61e2e8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_61e2e8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_61e2e8();
}
