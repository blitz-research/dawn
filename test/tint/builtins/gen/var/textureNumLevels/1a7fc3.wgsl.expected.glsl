#version 310 es

struct tint_symbol {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_ubo {
  uint texture_builtin_value_0;
} tint_symbol_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void textureNumLevels_1a7fc3() {
  uint res = tint_symbol_1.texture_builtin_value_0;
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureNumLevels_1a7fc3();
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

struct tint_symbol {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_ubo {
  uint texture_builtin_value_0;
} tint_symbol_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void textureNumLevels_1a7fc3() {
  uint res = tint_symbol_1.texture_builtin_value_0;
  prevent_dce.inner = res;
}

void fragment_main() {
  textureNumLevels_1a7fc3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct tint_symbol {
  uint texture_builtin_value_0;
};

layout(binding = 0, std140) uniform tint_symbol_ubo {
  uint texture_builtin_value_0;
} tint_symbol_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void textureNumLevels_1a7fc3() {
  uint res = tint_symbol_1.texture_builtin_value_0;
  prevent_dce.inner = res;
}

void compute_main() {
  textureNumLevels_1a7fc3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
