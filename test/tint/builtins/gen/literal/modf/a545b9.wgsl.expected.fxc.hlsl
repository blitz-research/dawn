SKIP: FAILED

struct modf_result_vec2_f16 {
  vector<float16_t, 2> fract;
  vector<float16_t, 2> whole;
};
void modf_a545b9() {
  modf_result_vec2_f16 res = {(float16_t(-0.5h)).xx, (float16_t(-1.0h)).xx};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_a545b9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_a545b9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_a545b9();
  return;
}
