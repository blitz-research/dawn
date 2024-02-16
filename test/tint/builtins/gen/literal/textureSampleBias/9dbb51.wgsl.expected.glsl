#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2DArray arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSampleBias_9dbb51() {
  vec4 res = textureOffset(arg_0_arg_1, vec3(vec2(1.0f), float(1)), ivec2(1), 1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleBias_9dbb51();
}

void main() {
  fragment_main();
  return;
}
