void clamp_23aa4f() {
  float res = 1.0f;
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  clamp_23aa4f();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  clamp_23aa4f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  clamp_23aa4f();
  return;
}
