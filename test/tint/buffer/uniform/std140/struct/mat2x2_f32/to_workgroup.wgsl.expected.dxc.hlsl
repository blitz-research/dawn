struct S {
  int before;
  float2x2 m;
  int after;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
groupshared S w[4];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float2x2 u_load_3(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

S u_load_1(uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 64u)) / 4;
  S tint_symbol_3 = {asint(u[scalar_offset_2 / 4][scalar_offset_2 % 4]), u_load_3((offset + 8u)), asint(u[scalar_offset_3 / 4][scalar_offset_3 % 4])};
  return tint_symbol_3;
}

typedef S u_load_ret[4];
u_load_ret u_load(uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 128u)));
    }
  }
  return arr;
}

void f_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      const uint i = idx;
      S tint_symbol_2 = (S)0;
      w[i] = tint_symbol_2;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  w = u_load(0u);
  w[1] = u_load_1(256u);
  w[3].m = u_load_3(264u);
  w[1].m[0] = asfloat(u[1].xy).yx;
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
