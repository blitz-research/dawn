#version 310 es

struct S_atomic {
  int x;
  uint a[10];
  uint y;
};

struct S {
  int x;
  uint a[10];
  uint y;
};

uint local_invocation_index_1 = 0u;
shared S_atomic wg;
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  wg.x = 0;
  wg.y = 0u;
  idx = local_invocation_index_2;
  while (true) {
    uint x_30 = idx;
    if (!((x_30 < 10u))) {
      break;
    }
    uint x_35 = idx;
    atomicExchange(wg.a[x_35], 0u);
    {
      uint x_41 = idx;
      idx = (x_41 + 1u);
    }
  }
  barrier();
  atomicExchange(wg.a[4], 1u);
  return;
}

void compute_main_1() {
  uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  {
    wg.x = 0;
    wg.y = 0u;
  }
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 10u); idx_1 = (idx_1 + 1u)) {
      uint i = idx_1;
      atomicExchange(wg.a[i], 0u);
    }
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
