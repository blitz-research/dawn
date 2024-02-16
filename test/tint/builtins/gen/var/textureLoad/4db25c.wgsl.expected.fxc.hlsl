Texture2DMS<float4> arg_0 : register(t0, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureLoad_4db25c() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  float res = arg_0.Load(uint3(arg_1, uint(0)), arg_2).x;
  prevent_dce.Store(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_4db25c();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_4db25c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_4db25c();
  return;
}
