RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_3fb31f() {
  uint3 arg_1 = (1u).xxx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_3fb31f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_3fb31f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_3fb31f();
  return;
}
