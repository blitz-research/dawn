void select_17441a() {
  bool arg_2 = true;
  float4 res = (arg_2 ? (1.0f).xxxx : (1.0f).xxxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_17441a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_17441a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_17441a();
  return;
}
