#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
  uint pad;
} prevent_dce;

void refract_cbc1d2() {
  vec3 arg_0 = vec3(1.0f);
  vec3 arg_1 = vec3(1.0f);
  float arg_2 = 1.0f;
  vec3 res = refract(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  refract_cbc1d2();
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

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
  uint pad;
} prevent_dce;

void refract_cbc1d2() {
  vec3 arg_0 = vec3(1.0f);
  vec3 arg_1 = vec3(1.0f);
  float arg_2 = 1.0f;
  vec3 res = refract(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

void fragment_main() {
  refract_cbc1d2();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
  uint pad;
} prevent_dce;

void refract_cbc1d2() {
  vec3 arg_0 = vec3(1.0f);
  vec3 arg_1 = vec3(1.0f);
  float arg_2 = 1.0f;
  vec3 res = refract(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

void compute_main() {
  refract_cbc1d2();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
