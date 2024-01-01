cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);

[numthreads(1, 1, 1)]
void main() {
  const int x = asint(u[0].x);
  s.Store(0u, asuint(x));
  return;
}
