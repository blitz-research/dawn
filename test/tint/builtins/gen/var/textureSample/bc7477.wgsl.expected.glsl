SKIP: FAILED

#version 310 es
precision highp float;
precision highp int;

uniform highp samplerCubeArray arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_bc7477() {
  vec3 arg_2 = vec3(1.0f);
  uint arg_3 = 1u;
  vec4 res = texture(arg_0_arg_1, vec4(arg_2, float(arg_3)));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_bc7477();
}

void main() {
  fragment_main();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:5: 'samplerCubeArray' : Reserved word. 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



