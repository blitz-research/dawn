SKIP: FAILED

#version 310 es

layout(r8) uniform highp writeonly image2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_35a5e2() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, ivec2(arg_1, 0));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureLoad_35a5e2();
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
error: Error parsing GLSL shader:
ERROR: 0:3: 'image load-store format' : not supported with this profile: es
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision highp float;
precision highp int;

layout(r8) uniform highp writeonly image2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_35a5e2() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, ivec2(arg_1, 0));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureLoad_35a5e2();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'image load-store format' : not supported with this profile: es
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

layout(r8) uniform highp writeonly image2D arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureLoad_35a5e2() {
  int arg_1 = 1;
  vec4 res = imageLoad(arg_0, ivec2(arg_1, 0));
  prevent_dce.inner = res;
}

void compute_main() {
  textureLoad_35a5e2();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:3: 'image load-store format' : not supported with this profile: es
ERROR: 0:3: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



