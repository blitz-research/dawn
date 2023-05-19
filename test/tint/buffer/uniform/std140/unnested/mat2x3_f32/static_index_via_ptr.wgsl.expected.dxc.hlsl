cbuffer cbuffer_m : register(b0) {
  uint4 m[2];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float2x3 m_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(m[scalar_offset / 4].xyz), asfloat(m[scalar_offset_1 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  const float2x3 l_m = m_load(0u);
  const float3 l_m_1 = asfloat(m[1].xyz);
  return;
}
