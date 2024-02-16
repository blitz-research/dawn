Texture2D<uint4> arg_0 : register(t0, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureLoad_ecc823() {
  int2 arg_1 = (1).xx;
  uint4 res = arg_0.Load(int3(arg_1, 0));
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureLoad_ecc823();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureLoad_ecc823();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureLoad_ecc823();
  return;
}
