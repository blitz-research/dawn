RWByteAddressBuffer prevent_dce : register(u0, space2);

void cosh_3b7bbf() {
  vector<float16_t, 4> res = (float16_t(1.0h)).xxxx;
  prevent_dce.Store<vector<float16_t, 4> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  cosh_3b7bbf();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  cosh_3b7bbf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cosh_3b7bbf();
  return;
}
