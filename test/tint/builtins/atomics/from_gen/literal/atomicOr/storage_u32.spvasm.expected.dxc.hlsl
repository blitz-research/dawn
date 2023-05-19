RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicOr(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedOr(offset, value, original_value);
  return original_value;
}


void atomicOr_5e95d4() {
  uint res = 0u;
  const uint x_9 = sb_rwatomicOr(0u, 1u);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicOr_5e95d4();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicOr_5e95d4();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
