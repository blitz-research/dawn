RWTexture2DArray<int4> arg_0 : register(u0, space1);

void textureStore_90a553() {
  arg_0[int3((1).xx, int(1u))] = (1).xxxx;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_90a553();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_90a553();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_90a553();
  return;
}
