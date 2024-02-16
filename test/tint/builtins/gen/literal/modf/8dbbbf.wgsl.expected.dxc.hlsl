struct modf_result_f16 {
  float16_t fract;
  float16_t whole;
};
void modf_8dbbbf() {
  modf_result_f16 res = {float16_t(-0.5h), float16_t(-1.0h)};
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  modf_8dbbbf();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  modf_8dbbbf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_8dbbbf();
  return;
}
