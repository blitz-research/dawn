RWByteAddressBuffer sb_rw : register(u0);

uint sb_rwatomicAnd(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedAnd(offset, value, original_value);
  return original_value;
}


void atomicAnd_85a8d9() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint x_18 = arg_1;
  uint x_13 = sb_rwatomicAnd(0u, x_18);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicAnd_85a8d9();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicAnd_85a8d9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
