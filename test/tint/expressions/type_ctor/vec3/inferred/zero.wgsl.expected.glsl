#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
vec3 f = vec3(0.0f);
ivec3 i = ivec3(0);
uvec3 u = uvec3(0u);
