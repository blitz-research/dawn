RWByteAddressBuffer prevent_dce : register(u0, space2);

void sign_9603b1() {
  int3 arg_0 = (1).xxx;
  int3 res = int3(sign(arg_0));
  prevent_dce.Store3(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  sign_9603b1();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  sign_9603b1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_9603b1();
  return;
}
