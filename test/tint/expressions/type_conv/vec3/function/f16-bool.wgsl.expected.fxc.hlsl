SKIP: FAILED

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t t = float16_t(0.0h);

vector<float16_t, 3> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 3>((t).xxx);
}

void f() {
  const vector<float16_t, 3> tint_symbol = m();
  bool3 v = bool3(tint_symbol);
}
