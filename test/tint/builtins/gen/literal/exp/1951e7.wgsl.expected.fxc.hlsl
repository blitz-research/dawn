RWByteAddressBuffer prevent_dce : register(u0, space2);

void exp_1951e7() {
  float2 res = (2.71828174591064453125f).xx;
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  exp_1951e7();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  exp_1951e7();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_1951e7();
  return;
}
