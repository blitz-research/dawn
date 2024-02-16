struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
groupshared uint arg_0;

void tint_zero_workgroup_memory(uint local_idx) {
  {
    uint atomic_result = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result);
  }
  GroupMemoryBarrierWithGroupSync();
}

void atomicCompareExchangeWeak_83580d() {
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  atomic_compare_exchange_result_u32 atomic_result_1 = (atomic_compare_exchange_result_u32)0;
  uint atomic_compare_value = arg_1;
  InterlockedCompareExchange(arg_0, atomic_compare_value, arg_2, atomic_result_1.old_value);
  atomic_result_1.exchanged = atomic_result_1.old_value == atomic_compare_value;
  atomic_compare_exchange_result_u32 res = atomic_result_1;
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void compute_main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicCompareExchangeWeak_83580d();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
