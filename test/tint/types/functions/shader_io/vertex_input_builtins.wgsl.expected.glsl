#version 310 es

struct PushConstants {
  uint first_instance;
};

layout(location=0) uniform PushConstants push_constants;
vec4 tint_symbol(uint vertex_index, uint instance_index) {
  uint foo = (vertex_index + (instance_index + push_constants.first_instance));
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = tint_symbol(uint(gl_VertexID), uint(gl_InstanceID));
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
