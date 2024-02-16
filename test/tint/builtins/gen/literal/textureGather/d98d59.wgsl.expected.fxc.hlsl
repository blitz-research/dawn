TextureCubeArray<float4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureGather_d98d59() {
  float4 res = arg_1.GatherGreen(arg_2, float4((1.0f).xxx, float(1)));
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureGather_d98d59();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureGather_d98d59();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureGather_d98d59();
  return;
}
