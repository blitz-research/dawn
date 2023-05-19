Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureGather_d90605() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = 1;
  float4 res = arg_0.Gather(arg_1, float3(arg_2, float(arg_3)), (1).xx);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureGather_d90605();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureGather_d90605();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureGather_d90605();
  return;
}
