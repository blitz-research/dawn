RWByteAddressBuffer prevent_dce : register(u0, space2);

void radians_61687a() {
  float2 res = (0.01745329238474369049f).xx;
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  radians_61687a();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  radians_61687a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  radians_61687a();
  return;
}
