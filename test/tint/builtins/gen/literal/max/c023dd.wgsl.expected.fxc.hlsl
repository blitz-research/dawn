void max_c023dd() {
  float res = 1.0f;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  max_c023dd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  max_c023dd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  max_c023dd();
  return;
}
