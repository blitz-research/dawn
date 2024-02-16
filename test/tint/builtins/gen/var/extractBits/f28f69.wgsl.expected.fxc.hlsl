uint2 tint_extract_bits(uint2 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  uint shl = (32u - e);
  uint shr = (shl + s);
  uint2 shl_result = ((shl < 32u) ? (v << uint2((shl).xx)) : (0u).xx);
  return ((shr < 32u) ? (shl_result >> uint2((shr).xx)) : ((shl_result >> (31u).xx) >> (1u).xx));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void extractBits_f28f69() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint2 res = tint_extract_bits(arg_0, arg_1, arg_2);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  extractBits_f28f69();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  extractBits_f28f69();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  extractBits_f28f69();
  return;
}
