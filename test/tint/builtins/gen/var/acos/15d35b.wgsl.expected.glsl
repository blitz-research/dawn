#version 310 es

void acos_15d35b() {
  vec2 res = vec2(0.25f);
}

vec4 vertex_main() {
  acos_15d35b();
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

void acos_15d35b() {
  vec2 res = vec2(0.25f);
}

void fragment_main() {
  acos_15d35b();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void acos_15d35b() {
  vec2 res = vec2(0.25f);
}

void compute_main() {
  acos_15d35b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
