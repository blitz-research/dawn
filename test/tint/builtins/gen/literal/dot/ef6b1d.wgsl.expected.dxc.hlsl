RWByteAddressBuffer prevent_dce : register(u0, space2);

void dot_ef6b1d() {
  int res = 4;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  dot_ef6b1d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  dot_ef6b1d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  dot_ef6b1d();
  return;
}
