RWByteAddressBuffer prevent_dce : register(u0, space2);

void faceForward_b316e5() {
  float4 arg_0 = (1.0f).xxxx;
  float4 arg_1 = (1.0f).xxxx;
  float4 arg_2 = (1.0f).xxxx;
  float4 res = faceforward(arg_0, arg_1, arg_2);
  prevent_dce.Store4(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  faceForward_b316e5();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  const float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  faceForward_b316e5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  faceForward_b316e5();
  return;
}
