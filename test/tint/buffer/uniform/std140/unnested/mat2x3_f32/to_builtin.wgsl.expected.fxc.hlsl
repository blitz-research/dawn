cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};

float2x3 u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x3(asfloat(u[scalar_offset / 4].xyz), asfloat(u[scalar_offset_1 / 4].xyz));
}

[numthreads(1, 1, 1)]
void f() {
  const float3x2 t = transpose(u_load(0u));
  const float l = length(asfloat(u[1].xyz));
  const float a = abs(asfloat(u[0].xyz).zxy.x);
  return;
}
