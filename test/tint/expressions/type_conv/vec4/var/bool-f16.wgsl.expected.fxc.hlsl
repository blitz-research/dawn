SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool4 u = (true).xxxx;

void f() {
  const vector<float16_t, 4> v = vector<float16_t, 4>(u);
}
