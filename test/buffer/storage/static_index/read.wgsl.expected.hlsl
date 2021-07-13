struct Inner {
  int x;
};
struct tint_padded_array_element {
  Inner el;
};

float2x3 tint_symbol_6(ByteAddressBuffer buffer, uint offset) {
  return float2x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))));
}

float3x2 tint_symbol_8(ByteAddressBuffer buffer, uint offset) {
  return float3x2(asfloat(buffer.Load2((offset + 0u))), asfloat(buffer.Load2((offset + 8u))), asfloat(buffer.Load2((offset + 16u))));
}

Inner tint_symbol_9(ByteAddressBuffer buffer, uint offset) {
  const Inner tint_symbol_11 = {asint(buffer.Load((offset + 0u)))};
  return tint_symbol_11;
}

typedef tint_padded_array_element tint_symbol_10_ret[4];
tint_symbol_10_ret tint_symbol_10(ByteAddressBuffer buffer, uint offset) {
  const tint_padded_array_element tint_symbol_12[4] = {{tint_symbol_9(buffer, (offset + 0u))}, {tint_symbol_9(buffer, (offset + 16u))}, {tint_symbol_9(buffer, (offset + 32u))}, {tint_symbol_9(buffer, (offset + 48u))}};
  return tint_symbol_12;
}

ByteAddressBuffer s : register(t0, space0);

[numthreads(1, 1, 1)]
void main() {
  const int3 a = asint(s.Load3(0u));
  const int b = asint(s.Load(12u));
  const uint3 c = s.Load3(16u);
  const uint d = s.Load(28u);
  const float3 e = asfloat(s.Load3(32u));
  const float f = asfloat(s.Load(44u));
  const float2x3 g = tint_symbol_6(s, 48u);
  const float3x2 h = tint_symbol_8(s, 80u);
  const Inner i = tint_symbol_9(s, 104u);
  const tint_padded_array_element j[4] = tint_symbol_10(s, 108u);
  return;
}
