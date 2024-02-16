groupshared float2x4 S;

void tint_zero_workgroup_memory(uint local_idx) {
  {
    S = float2x4((0.0f).xxxx, (0.0f).xxxx);
  }
  GroupMemoryBarrierWithGroupSync();
}

float4 func_S_X(uint pointer[1]) {
  return S[pointer[0]];
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint tint_symbol_2[1] = {1u};
  float4 r = func_S_X(tint_symbol_2);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
