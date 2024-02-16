#version 310 es

shared float w;
void tint_zero_workgroup_memory(uint local_idx) {
  {
    w = 0.0f;
  }
  barrier();
}

float p = 0.0f;
void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  float x = (p + w);
  p = x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
