void ldexp_fdbc7b() {
  float res = 2.0f;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  ldexp_fdbc7b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  ldexp_fdbc7b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_fdbc7b();
  return;
}
