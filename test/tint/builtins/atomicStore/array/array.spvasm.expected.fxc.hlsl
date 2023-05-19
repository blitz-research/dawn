static uint local_invocation_index_1 = 0u;
groupshared uint wg[4];

void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  while (true) {
    const uint x_21 = idx;
    if (!((x_21 < 4u))) {
      break;
    }
    const uint x_26 = idx;
    uint atomic_result = 0u;
    InterlockedExchange(wg[x_26], 0u, atomic_result);
    {
      const uint x_33 = idx;
      idx = (x_33 + 1u);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg[1], 1u, atomic_result_1);
  return;
}

void compute_main_1() {
  const uint x_47 = local_invocation_index_1;
  compute_main_inner(x_47);
  return;
}

struct tint_symbol_1 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 4u); idx_1 = (idx_1 + 1u)) {
      const uint i = idx_1;
      uint atomic_result_2 = 0u;
      InterlockedExchange(wg[i], 0u, atomic_result_2);
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
