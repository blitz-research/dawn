RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicOr(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedOr(offset, value, original_value);
  return original_value;
}


void atomicOr_8d96a0() {
  int res = 0;
  const int x_9 = sb_rwatomicOr(0u, 1);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicOr_8d96a0();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicOr_8d96a0();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
