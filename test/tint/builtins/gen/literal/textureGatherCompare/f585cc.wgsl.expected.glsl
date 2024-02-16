#version 310 es

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureGatherCompare_f585cc() {
  vec4 res = textureGatherOffset(arg_0_arg_1, vec3(vec2(1.0f), float(1)), 1.0f, ivec2(1));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureGatherCompare_f585cc();
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

void textureGatherCompare_f585cc() {
  vec4 res = textureGatherOffset(arg_0_arg_1, vec3(vec2(1.0f), float(1)), 1.0f, ivec2(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureGatherCompare_f585cc();
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

void textureGatherCompare_f585cc() {
  vec4 res = textureGatherOffset(arg_0_arg_1, vec3(vec2(1.0f), float(1)), 1.0f, ivec2(1));
  prevent_dce.inner = res;
}

void compute_main() {
  textureGatherCompare_f585cc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
