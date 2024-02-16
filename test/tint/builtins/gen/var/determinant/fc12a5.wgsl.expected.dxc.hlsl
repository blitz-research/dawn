RWByteAddressBuffer prevent_dce : register(u0, space2);

void determinant_fc12a5() {
  matrix<float16_t, 2, 2> arg_0 = matrix<float16_t, 2, 2>((float16_t(1.0h)).xx, (float16_t(1.0h)).xx);
  float16_t res = determinant(arg_0);
  prevent_dce.Store<float16_t>(0u, res);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  determinant_fc12a5();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  determinant_fc12a5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  determinant_fc12a5();
  return;
}
