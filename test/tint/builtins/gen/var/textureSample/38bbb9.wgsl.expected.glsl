#version 310 es
precision highp float;
precision highp int;

uniform highp sampler2DShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSample_38bbb9() {
  vec2 arg_2 = vec2(1.0f);
  float res = texture(arg_0_arg_1, vec3(arg_2, 0.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_38bbb9();
}

void main() {
  fragment_main();
  return;
}
