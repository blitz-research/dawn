SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static uint2 u = (1u).xx;

void f() {
  const vector<float16_t, 2> v = vector<float16_t, 2>(u);
}
