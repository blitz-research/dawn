void select_2c96d4() {
  bool3 arg_2 = (true).xxx;
  float3 res = (arg_2 ? (1.0f).xxx : (1.0f).xxx);
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_2c96d4();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_2c96d4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_2c96d4();
  return;
}
