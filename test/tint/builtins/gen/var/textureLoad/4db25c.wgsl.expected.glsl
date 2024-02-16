#version 310 es

uniform highp sampler2DMS arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureLoad_4db25c() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  float res = texelFetch(arg_0_1, ivec2(arg_1), int(arg_2)).x;
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_4db25c();
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

uniform highp sampler2DMS arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureLoad_4db25c() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  float res = texelFetch(arg_0_1, ivec2(arg_1), int(arg_2)).x;
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_4db25c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler2DMS arg_0_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureLoad_4db25c() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  float res = texelFetch(arg_0_1, ivec2(arg_1), int(arg_2)).x;
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_4db25c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
