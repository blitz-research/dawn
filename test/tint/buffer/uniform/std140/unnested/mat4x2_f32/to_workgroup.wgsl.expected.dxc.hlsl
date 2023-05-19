cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
groupshared float4x2 w;

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

float4x2 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = u[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_3 = u[scalar_offset_3 / 4];
  return float4x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
}

void f_inner(uint local_invocation_index) {
  {
    w = float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = u_load(0u);
  w[1] = asfloat(u[0].xy);
  w[1] = asfloat(u[0].xy).yx;
  w[0][1] = asfloat(u[0].z);
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
