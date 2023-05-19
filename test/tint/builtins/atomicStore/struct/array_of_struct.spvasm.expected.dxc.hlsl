struct S_atomic {
  int x;
  uint a;
  uint y;
};

static uint local_invocation_index_1 = 0u;
groupshared S_atomic wg[10];

void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  while (true) {
    const uint x_23 = idx;
    if (!((x_23 < 10u))) {
      break;
    }
    const uint x_28 = idx;
    wg[x_28].x = 0;
    uint atomic_result = 0u;
    InterlockedExchange(wg[x_28].a, 0u, atomic_result);
    wg[x_28].y = 0u;
    {
      const uint x_41 = idx;
      idx = (x_41 + 1u);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg[4].a, 1u, atomic_result_1);
  return;
}

void compute_main_1() {
  const uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
  return;
}

struct tint_symbol_1 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 10u); idx_1 = (idx_1 + 1u)) {
      const uint i = idx_1;
      wg[i].x = 0;
      uint atomic_result_2 = 0u;
      InterlockedExchange(wg[i].a, 0u, atomic_result_2);
      wg[i].y = 0u;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner_1(tint_symbol.local_invocation_index_1_param);
  return;
}
