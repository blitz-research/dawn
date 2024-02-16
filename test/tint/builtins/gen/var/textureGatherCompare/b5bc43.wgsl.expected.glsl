#version 310 es

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureGatherCompare_b5bc43() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec4 res = textureGather(arg_0_arg_1, vec3(arg_2, float(arg_3)), arg_4);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureGatherCompare_b5bc43();
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
precision highp float;
precision highp int;

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureGatherCompare_b5bc43() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec4 res = textureGather(arg_0_arg_1, vec3(arg_2, float(arg_3)), arg_4);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureGatherCompare_b5bc43();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureGatherCompare_b5bc43() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  vec4 res = textureGather(arg_0_arg_1, vec3(arg_2, float(arg_3)), arg_4);
  prevent_dce.inner = res;
}

void compute_main() {
  textureGatherCompare_b5bc43();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
