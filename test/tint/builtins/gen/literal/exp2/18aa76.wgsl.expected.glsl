#version 310 es

void exp2_18aa76() {
  vec2 res = vec2(2.0f);
}

vec4 vertex_main() {
  exp2_18aa76();
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

void exp2_18aa76() {
  vec2 res = vec2(2.0f);
}

void fragment_main() {
  exp2_18aa76();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void exp2_18aa76() {
  vec2 res = vec2(2.0f);
}

void compute_main() {
  exp2_18aa76();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
