void ldexp_4a3ad9() {
  float3 res = (2.0f).xxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_4a3ad9();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_4a3ad9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_4a3ad9();
  return;
}
