#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void select_fb7e53() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bool arg_2 = true;
  bvec2 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.inner = (all(equal(res, bvec2(false))) ? 1 : 0);
}

vec4 vertex_main() {
  select_fb7e53();
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
  int inner;
} prevent_dce;

void select_fb7e53() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bool arg_2 = true;
  bvec2 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.inner = (all(equal(res, bvec2(false))) ? 1 : 0);
}

void fragment_main() {
  select_fb7e53();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void select_fb7e53() {
  bvec2 arg_0 = bvec2(true);
  bvec2 arg_1 = bvec2(true);
  bool arg_2 = true;
  bvec2 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.inner = (all(equal(res, bvec2(false))) ? 1 : 0);
}

void compute_main() {
  select_fb7e53();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
