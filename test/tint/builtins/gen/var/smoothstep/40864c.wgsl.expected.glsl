#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void smoothstep_40864c() {
  vec4 arg_0 = vec4(2.0f);
  vec4 arg_1 = vec4(4.0f);
  vec4 arg_2 = vec4(3.0f);
  vec4 res = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  smoothstep_40864c();
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

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void smoothstep_40864c() {
  vec4 arg_0 = vec4(2.0f);
  vec4 arg_1 = vec4(4.0f);
  vec4 arg_2 = vec4(3.0f);
  vec4 res = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

void fragment_main() {
  smoothstep_40864c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void smoothstep_40864c() {
  vec4 arg_0 = vec4(2.0f);
  vec4 arg_1 = vec4(4.0f);
  vec4 arg_2 = vec4(3.0f);
  vec4 res = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

void compute_main() {
  smoothstep_40864c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
