RWByteAddressBuffer prevent_dce : register(u0, space2);

void exp_d98450() {
  float3 res = (2.71828174591064453125f).xxx;
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  exp_d98450();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  exp_d98450();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_d98450();
  return;
}
