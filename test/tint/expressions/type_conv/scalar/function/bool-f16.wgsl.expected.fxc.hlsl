SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool m() {
  t = true;
  return bool(t);
}

void f() {
  const bool tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
