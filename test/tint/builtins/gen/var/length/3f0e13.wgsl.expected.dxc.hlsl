RWByteAddressBuffer prevent_dce : register(u0, space2);

void length_3f0e13() {
  vector<float16_t, 2> arg_0 = (float16_t(0.0h)).xx;
  float16_t res = length(arg_0);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  length_3f0e13();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  length_3f0e13();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_3f0e13();
  return;
}
