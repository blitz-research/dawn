#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
  uint pad;
} prevent_dce;

void dpdyCoarse_ae1873() {
  vec3 res = dFdy(vec3(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdyCoarse_ae1873();
}

void main() {
  fragment_main();
  return;
}
