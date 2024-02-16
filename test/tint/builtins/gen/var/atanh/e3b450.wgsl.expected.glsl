#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_select(f16vec4 param_0, f16vec4 param_1, bvec4 param_2) {
    return f16vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


f16vec4 tint_atanh(f16vec4 x) {
  return tint_select(atanh(x), f16vec4(0.0hf), greaterThanEqual(x, f16vec4(1.0hf)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

void atanh_e3b450() {
  f16vec4 arg_0 = f16vec4(0.5hf);
  f16vec4 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  atanh_e3b450();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;
precision highp int;

f16vec4 tint_select(f16vec4 param_0, f16vec4 param_1, bvec4 param_2) {
    return f16vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


f16vec4 tint_atanh(f16vec4 x) {
  return tint_select(atanh(x), f16vec4(0.0hf), greaterThanEqual(x, f16vec4(1.0hf)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

void atanh_e3b450() {
  f16vec4 arg_0 = f16vec4(0.5hf);
  f16vec4 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  atanh_e3b450();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_select(f16vec4 param_0, f16vec4 param_1, bvec4 param_2) {
    return f16vec4(param_2[0] ? param_1[0] : param_0[0], param_2[1] ? param_1[1] : param_0[1], param_2[2] ? param_1[2] : param_0[2], param_2[3] ? param_1[3] : param_0[3]);
}


f16vec4 tint_atanh(f16vec4 x) {
  return tint_select(atanh(x), f16vec4(0.0hf), greaterThanEqual(x, f16vec4(1.0hf)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec4 inner;
} prevent_dce;

void atanh_e3b450() {
  f16vec4 arg_0 = f16vec4(0.5hf);
  f16vec4 res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  atanh_e3b450();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
