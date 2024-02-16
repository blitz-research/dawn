#version 310 es

shared ivec3 v;
void tint_zero_workgroup_memory(uint local_idx) {
  {
    v = ivec3(0);
  }
  barrier();
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
