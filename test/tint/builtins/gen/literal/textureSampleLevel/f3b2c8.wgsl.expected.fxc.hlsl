Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSampleLevel_f3b2c8() {
  float res = arg_0.SampleLevel(arg_1, (1.0f).xx, 1u, int2((1).xx)).x;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureSampleLevel_f3b2c8();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureSampleLevel_f3b2c8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_f3b2c8();
  return;
}
