#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  mat2x4 m = mat2x4(vec4(0.0f), vec4(0.0f));
  mat2x4 m_1 = mat2x4(m);
}
