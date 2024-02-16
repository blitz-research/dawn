float tint_atanh(float x) {
  return (log(((1.0f + x) / (1.0f - x))) * 0.5f);
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void atanh_7997d8() {
  float arg_0 = 0.5f;
  float res = tint_atanh(arg_0);
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  atanh_7997d8();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  atanh_7997d8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atanh_7997d8();
  return;
}
