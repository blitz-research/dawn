#version 310 es

shared int arg_0;
void tint_zero_workgroup_memory(uint local_idx) {
  {
    atomicExchange(arg_0, 0);
  }
  barrier();
}

void atomicStore_8bea94() {
  atomicExchange(arg_0, 1);
}

void compute_main(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicStore_8bea94();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
