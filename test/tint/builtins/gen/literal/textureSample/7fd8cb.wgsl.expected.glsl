SKIP: FAILED

#version 310 es
precision highp float;

uniform highp samplerCubeArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSample_7fd8cb() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), float(1u)), 0.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_7fd8cb();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: 'samplerCubeArrayShadow' : Reserved word. 
ERROR: 0:4: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



