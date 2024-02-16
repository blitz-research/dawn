#version 310 es

shared uint arg_0;
void tint_zero_workgroup_memory(uint local_idx) {
  {
    atomicExchange(arg_0, 0u);
  }
  barrier();
}

uint local_invocation_index_1 = 0u;
void atomicMin_69d383() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint x_18 = arg_1;
  uint x_14 = atomicMin(arg_0, x_18);
  res = x_14;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0u);
  barrier();
  atomicMin_69d383();
  return;
}

void compute_main_1() {
  uint x_32 = local_invocation_index_1;
  compute_main_inner(x_32);
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
