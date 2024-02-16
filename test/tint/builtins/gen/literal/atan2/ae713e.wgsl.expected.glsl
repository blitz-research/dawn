#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void atan2_ae713e() {
  vec4 res = vec4(0.78539818525314331055f);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  atan2_ae713e();
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
  vec4 inner;
} prevent_dce;

void atan2_ae713e() {
  vec4 res = vec4(0.78539818525314331055f);
  prevent_dce.inner = res;
}

void fragment_main() {
  atan2_ae713e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void atan2_ae713e() {
  vec4 res = vec4(0.78539818525314331055f);
  prevent_dce.inner = res;
}

void compute_main() {
  atan2_ae713e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
