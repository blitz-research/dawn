#version 310 es

shared int arg_0;
void tint_zero_workgroup_memory(uint local_idx) {
  {
    atomicExchange(arg_0, 0);
  }
  barrier();
}

uint local_invocation_index_1 = 0u;
void atomicStore_8bea94() {
  atomicExchange(arg_0, 1);
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0);
  barrier();
  atomicStore_8bea94();
  return;
}

void compute_main_1() {
  uint x_29 = local_invocation_index_1;
  compute_main_inner(x_29);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  tint_zero_workgroup_memory(local_invocation_index_1_param);
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
