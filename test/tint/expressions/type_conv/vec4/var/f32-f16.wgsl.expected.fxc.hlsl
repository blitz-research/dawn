SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float4 u = (1.0f).xxxx;

void f() {
  const vector<float16_t, 4> v = vector<float16_t, 4>(u);
}
