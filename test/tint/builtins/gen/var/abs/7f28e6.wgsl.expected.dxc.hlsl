RWByteAddressBuffer prevent_dce : register(u0, space2);

void abs_7f28e6() {
  uint2 arg_0 = (1u).xx;
  uint2 res = abs(arg_0);
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  abs_7f28e6();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  abs_7f28e6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  abs_7f28e6();
  return;
}
