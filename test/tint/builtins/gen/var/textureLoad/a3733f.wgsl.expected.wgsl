@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba16uint, read_write>;

fn textureLoad_a3733f() {
  var arg_1 = vec2<i32>(1i);
  var res : vec4<u32> = textureLoad(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_a3733f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_a3733f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_a3733f();
}
