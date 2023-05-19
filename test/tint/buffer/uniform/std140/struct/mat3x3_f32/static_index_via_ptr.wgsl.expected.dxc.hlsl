struct Inner {
  float3x3 m;
};
struct Outer {
  Inner a[4];
};

cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};

float3x3 a_load_4(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x3(asfloat(a[scalar_offset / 4].xyz), asfloat(a[scalar_offset_1 / 4].xyz), asfloat(a[scalar_offset_2 / 4].xyz));
}

Inner a_load_3(uint offset) {
  const Inner tint_symbol = {a_load_4((offset + 0u))};
  return tint_symbol;
}

typedef Inner a_load_2_ret[4];
a_load_2_ret a_load_2(uint offset) {
  Inner arr[4] = (Inner[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = a_load_3((offset + (i * 64u)));
    }
  }
  return arr;
}

Outer a_load_1(uint offset) {
  const Outer tint_symbol_1 = {a_load_2((offset + 0u))};
  return tint_symbol_1;
}

typedef Outer a_load_ret[4];
a_load_ret a_load(uint offset) {
  Outer arr_1[4] = (Outer[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = a_load_1((offset + (i_1 * 256u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void f() {
  const Outer l_a[4] = a_load(0u);
  const Outer l_a_3 = a_load_1(768u);
  const Inner l_a_3_a[4] = a_load_2(768u);
  const Inner l_a_3_a_2 = a_load_3(896u);
  const float3x3 l_a_3_a_2_m = a_load_4(896u);
  const float3 l_a_3_a_2_m_1 = asfloat(a[57].xyz);
  const float l_a_3_a_2_m_1_0 = asfloat(a[57].x);
  return;
}
