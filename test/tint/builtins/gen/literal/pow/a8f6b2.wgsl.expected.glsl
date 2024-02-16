#version 310 es

void pow_a8f6b2() {
  vec4 res = vec4(1.0f);
}

vec4 vertex_main() {
  pow_a8f6b2();
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

void pow_a8f6b2() {
  vec4 res = vec4(1.0f);
}

void fragment_main() {
  pow_a8f6b2();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void pow_a8f6b2() {
  vec4 res = vec4(1.0f);
}

void compute_main() {
  pow_a8f6b2();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
