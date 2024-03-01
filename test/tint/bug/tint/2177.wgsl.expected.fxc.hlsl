RWByteAddressBuffer arr : register(u0);

uint f2_arr() {
  uint tint_symbol_1 = 0u;
  arr.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = (tint_symbol_1 / 4u);
  return tint_symbol_2;
}

uint f1_arr() {
  return f2_arr();
}

uint f0_arr() {
  return f1_arr();
}

[numthreads(1, 1, 1)]
void main() {
  arr.Store(0u, asuint(f0_arr()));
  return;
}
