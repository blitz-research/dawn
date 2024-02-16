#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2D arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSampleBias_a161cf() {
  vec2 arg_2 = vec2(1.0f);
  float arg_3 = 1.0f;
  vec4 res = textureOffset(arg_0_arg_1, arg_2, ivec2(1), arg_3);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleBias_a161cf();
}

void main() {
  fragment_main();
  return;
}
