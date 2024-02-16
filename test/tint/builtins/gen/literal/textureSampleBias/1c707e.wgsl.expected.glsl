#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2DArray arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSampleBias_1c707e() {
  vec4 res = texture(arg_0_arg_1, vec3(vec2(1.0f), float(1u)), 1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleBias_1c707e();
}

void main() {
  fragment_main();
  return;
}
