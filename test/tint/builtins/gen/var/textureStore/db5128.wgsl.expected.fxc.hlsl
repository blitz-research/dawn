RWTexture2DArray<uint4> arg_0 : register(u0, space1);

void textureStore_db5128() {
  int2 arg_1 = (1).xx;
  uint arg_2 = 1u;
  uint4 arg_3 = (1u).xxxx;
  arg_0[int3(arg_1, int(arg_2))] = arg_3;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_db5128();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_db5128();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_db5128();
  return;
}
