Texture2DArray<float4> arg_1 : register(t1, space1);
SamplerState arg_2 : register(s2, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureGather_ea8eb4() {
  float4 res = arg_1.GatherGreen(arg_2, float3((1.0f).xx, float(1u)));
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureGather_ea8eb4();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureGather_ea8eb4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureGather_ea8eb4();
  return;
}
