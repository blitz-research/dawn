SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int u = 1;

void f() {
  const float16_t v = float16_t(u);
}
