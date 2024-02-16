RWByteAddressBuffer sb_rw : register(u0);

int sb_rwatomicSub(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedAdd(offset, -value, original_value);
  return original_value;
}


void atomicSub_051100() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_20 = arg_1;
  int x_13 = sb_rwatomicSub(0u, x_20);
  res = x_13;
  return;
}

void fragment_main_1() {
  atomicSub_051100();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicSub_051100();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
