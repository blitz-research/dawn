RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_80bf1d() {
  int2 arg_1 = (1).xx;
  int4 arg_2 = (1).xxxx;
  arg_0[arg_1] = arg_2;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureStore_80bf1d();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureStore_80bf1d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_80bf1d();
  return;
}
