#version 310 es

void inverseSqrt_6d0783() {
  vec3 res = vec3(1.0f);
}

vec4 vertex_main() {
  inverseSqrt_6d0783();
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

void inverseSqrt_6d0783() {
  vec3 res = vec3(1.0f);
}

void fragment_main() {
  inverseSqrt_6d0783();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

void inverseSqrt_6d0783() {
  vec3 res = vec3(1.0f);
}

void compute_main() {
  inverseSqrt_6d0783();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
