RWTexture3D<float4> arg_0 : register(u0, space1);

void textureStore_9aedd3() {
  uint3 arg_1 = (1u).xxx;
  float4 arg_2 = (1.0f).xxxx;
  arg_0[arg_1] = arg_2;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_9aedd3();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_9aedd3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9aedd3();
  return;
}
