@group(1) @binding(0) var arg_0 : texture_1d<f32>;

@group(1) @binding(1) var arg_1 : sampler;

fn textureSample_6e64fb() {
  var arg_2 = 1.0f;
  var res : vec4<f32> = textureSample(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@fragment
fn fragment_main() {
  textureSample_6e64fb();
}
