static float p = 0.0f;
groupshared float w;
ByteAddressBuffer uniforms : register(t1);
RWByteAddressBuffer storages : register(u0);

void no_uses() {
}

void zoo() {
  p = (p * 2.0f);
}

void bar(float a, float b) {
  p = a;
  w = b;
  storages.Store(0u, asuint(asfloat(uniforms.Load(0u))));
  zoo();
}

void foo(float a) {
  const float b = 2.0f;
  bar(a, b);
  no_uses();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  {
    w = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
  foo(1.0f);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
