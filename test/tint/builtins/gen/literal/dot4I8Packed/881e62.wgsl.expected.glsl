#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot4I8Packed_881e62() {
  int res = 1;
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  dot4I8Packed_881e62();
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

void dot4I8Packed_881e62() {
  int res = 1;
  prevent_dce.inner = res;
}

void fragment_main() {
  dot4I8Packed_881e62();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot4I8Packed_881e62() {
  int res = 1;
  prevent_dce.inner = res;
}

void compute_main() {
  dot4I8Packed_881e62();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
