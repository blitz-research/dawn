RWByteAddressBuffer prevent_dce : register(u0, space2);

void acos_489247() {
  float arg_0 = 0.96891242265701293945f;
  float res = acos(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  acos_489247();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  acos_489247();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  acos_489247();
  return;
}
