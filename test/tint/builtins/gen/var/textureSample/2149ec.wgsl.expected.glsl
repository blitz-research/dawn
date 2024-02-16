#version 310 es
precision highp float;
precision highp int;

uniform highp sampler3D arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_2149ec() {
  vec3 arg_2 = vec3(1.0f);
  vec4 res = textureOffset(arg_0_arg_1, arg_2, ivec3(1));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_2149ec();
}

void main() {
  fragment_main();
  return;
}
