RWByteAddressBuffer prevent_dce : register(u0, space2);

void select_c41bd1() {
  bool4 arg_0 = (true).xxxx;
  bool4 arg_1 = (true).xxxx;
  bool arg_2 = true;
  bool4 res = (arg_2 ? arg_1 : arg_0);
  prevent_dce.Store(0u, asuint((all((res == (false).xxxx)) ? 1 : 0)));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  select_c41bd1();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  select_c41bd1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  select_c41bd1();
  return;
}
