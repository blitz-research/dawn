#version 310 es

float tint_atanh(float x) {
  return ((x >= 1.0f) ? 0.0f : atanh(x));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void atanh_7997d8() {
  float arg_0 = 0.5f;
  float res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  atanh_7997d8();
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

float tint_atanh(float x) {
  return ((x >= 1.0f) ? 0.0f : atanh(x));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void atanh_7997d8() {
  float arg_0 = 0.5f;
  float res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  atanh_7997d8();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

float tint_atanh(float x) {
  return ((x >= 1.0f) ? 0.0f : atanh(x));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void atanh_7997d8() {
  float arg_0 = 0.5f;
  float res = tint_atanh(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  atanh_7997d8();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
