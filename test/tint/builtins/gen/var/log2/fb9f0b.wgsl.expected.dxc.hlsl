RWByteAddressBuffer prevent_dce : register(u0, space2);

void log2_fb9f0b() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> res = log2(arg_0);
  prevent_dce.Store<vector<float16_t, 2> >(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  log2_fb9f0b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  log2_fb9f0b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  log2_fb9f0b();
  return;
}
