#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_degrees(float16_t param_0) {
  return param_0 * 57.29577951308232286465hf;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void degrees_5e9805() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tint_degrees(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  degrees_5e9805();
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

float16_t tint_degrees(float16_t param_0) {
  return param_0 * 57.29577951308232286465hf;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void degrees_5e9805() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tint_degrees(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  degrees_5e9805();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

float16_t tint_degrees(float16_t param_0) {
  return param_0 * 57.29577951308232286465hf;
}


layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float16_t inner;
} prevent_dce;

void degrees_5e9805() {
  float16_t arg_0 = 1.0hf;
  float16_t res = tint_degrees(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  degrees_5e9805();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
