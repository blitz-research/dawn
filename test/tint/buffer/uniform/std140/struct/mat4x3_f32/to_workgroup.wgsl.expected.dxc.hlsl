struct S {
  int before;
  float4x3 m;
  int after;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[48];
};
groupshared S w[4];

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float4x3 u_load_3(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz), asfloat(u[scalar_offset_2 / 4].xyz), asfloat(u[scalar_offset_3 / 4].xyz));
}

S u_load_1(uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 128u)) / 4;
  S tint_symbol_3 = {asint(u[scalar_offset_4 / 4][scalar_offset_4 % 4]), u_load_3((offset + 16u)), asint(u[scalar_offset_5 / 4][scalar_offset_5 % 4])};
  return tint_symbol_3;
}

typedef S u_load_ret[4];
u_load_ret u_load(uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 192u)));
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
  w[1] = u_load_1(384u);
  w[3].m = u_load_3(400u);
  w[1].m[0] = asfloat(u[2].xyz).zxy;
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
