RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthCoarse_1e59d9() {
  float3 res = fwidth((1.0f).xxx);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  fwidthCoarse_1e59d9();
  return;
}
