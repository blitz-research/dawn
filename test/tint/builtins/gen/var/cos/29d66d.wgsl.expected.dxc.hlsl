RWByteAddressBuffer prevent_dce : register(u0, space2);

void cos_29d66d() {
  float4 arg_0 = (0.0f).xxxx;
  float4 res = cos(arg_0);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  cos_29d66d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  cos_29d66d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cos_29d66d();
  return;
}
