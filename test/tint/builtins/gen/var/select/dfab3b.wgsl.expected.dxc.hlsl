void select_dfab3b() {
  bool arg_2 = true;
  int3 res = (arg_2 ? (1).xxx : (1).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_dfab3b();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_dfab3b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_dfab3b();
  return;
}
