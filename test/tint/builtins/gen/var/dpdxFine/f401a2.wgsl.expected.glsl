#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void dpdxFine_f401a2() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdxFine_f401a2();
}

void main() {
  fragment_main();
  return;
}
