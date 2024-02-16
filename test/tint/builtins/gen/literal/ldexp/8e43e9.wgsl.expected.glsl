#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec3 inner;
} prevent_dce;

void ldexp_8e43e9() {
  f16vec3 res = f16vec3(2.0hf);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  ldexp_8e43e9();
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

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec3 inner;
} prevent_dce;

void ldexp_8e43e9() {
  f16vec3 res = f16vec3(2.0hf);
  prevent_dce.inner = res;
}

void fragment_main() {
  ldexp_8e43e9();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  f16vec3 inner;
} prevent_dce;

void ldexp_8e43e9() {
  f16vec3 res = f16vec3(2.0hf);
  prevent_dce.inner = res;
}

void compute_main() {
  ldexp_8e43e9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
