RWByteAddressBuffer prevent_dce : register(u0, space2);

void unpack4x8unorm_750c74() {
  float4 res = float4(0.0039215688593685627f, 0.0f, 0.0f, 0.0f);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  unpack4x8unorm_750c74();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  unpack4x8unorm_750c74();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack4x8unorm_750c74();
  return;
}
