Texture3D<int4> arg_0 : register(t0, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureLoad_8a9988() {
  uint3 arg_1 = (1u).xxx;
  int4 res = arg_0.Load(uint4(arg_1, uint(0)));
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_8a9988();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_8a9988();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_8a9988();
  return;
}
