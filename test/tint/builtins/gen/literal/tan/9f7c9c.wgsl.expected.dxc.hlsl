RWByteAddressBuffer prevent_dce : register(u0, space2);

void tan_9f7c9c() {
  vector<float16_t, 2> res = (float16_t(1.556640625h)).xx;
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  tan_9f7c9c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  tan_9f7c9c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tan_9f7c9c();
  return;
}
