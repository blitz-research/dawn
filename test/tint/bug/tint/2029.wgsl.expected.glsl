#version 310 es

layout(binding = 0, std430) buffer s_block_ssbo {
  ivec3 inner;
  uint pad;
} s;

void tint_symbol() {
  s.inner = ivec3(1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
