SKIP: FAILED

#version 310 es

layout(rgba8) uniform highp writeonly image2D t_rgba8unorm;
layout(rgba8_snorm) uniform highp writeonly image2D t_rgba8snorm;
layout(rgba8ui) uniform highp writeonly uimage2D t_rgba8uint;
layout(rgba8i) uniform highp writeonly iimage2D t_rgba8sint;
layout(rgba16ui) uniform highp writeonly uimage2D t_rgba16uint;
layout(rgba16i) uniform highp writeonly iimage2D t_rgba16sint;
layout(rgba16f) uniform highp writeonly image2D t_rgba16float;
layout(r32ui) uniform highp writeonly uimage2D t_r32uint;
layout(r32i) uniform highp writeonly iimage2D t_r32sint;
layout(r32f) uniform highp writeonly image2D t_r32float;
layout(rg32ui) uniform highp writeonly uimage2D t_rg32uint;
layout(rg32i) uniform highp writeonly iimage2D t_rg32sint;
layout(rg32f) uniform highp writeonly image2D t_rg32float;
layout(rgba32ui) uniform highp writeonly uimage2D t_rgba32uint;
layout(rgba32i) uniform highp writeonly iimage2D t_rgba32sint;
layout(rgba32f) uniform highp writeonly image2D t_rgba32float;
void tint_symbol() {
  uint dim1 = uvec2(imageSize(t_rgba8unorm)).x;
  uint dim2 = uvec2(imageSize(t_rgba8snorm)).x;
  uint dim3 = uvec2(imageSize(t_rgba8uint)).x;
  uint dim4 = uvec2(imageSize(t_rgba8sint)).x;
  uint dim5 = uvec2(imageSize(t_rgba16uint)).x;
  uint dim6 = uvec2(imageSize(t_rgba16sint)).x;
  uint dim7 = uvec2(imageSize(t_rgba16float)).x;
  uint dim8 = uvec2(imageSize(t_r32uint)).x;
  uint dim9 = uvec2(imageSize(t_r32sint)).x;
  uint dim10 = uvec2(imageSize(t_r32float)).x;
  uint dim11 = uvec2(imageSize(t_rg32uint)).x;
  uint dim12 = uvec2(imageSize(t_rg32sint)).x;
  uint dim13 = uvec2(imageSize(t_rg32float)).x;
  uint dim14 = uvec2(imageSize(t_rgba32uint)).x;
  uint dim15 = uvec2(imageSize(t_rgba32sint)).x;
  uint dim16 = uvec2(imageSize(t_rgba32float)).x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
error: Error parsing GLSL shader:
ERROR: 0:13: 'image load-store format' : not supported with this profile: es
ERROR: 0:13: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



