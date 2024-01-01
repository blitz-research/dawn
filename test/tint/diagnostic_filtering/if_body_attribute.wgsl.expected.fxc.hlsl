<dawn>/test/tint/diagnostic_filtering/if_body_attribute.wgsl:7:9 warning: 'textureSample' must only be called from uniform control flow
    _ = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

<dawn>/test/tint/diagnostic_filtering/if_body_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  if (x > 0) @diagnostic(warning, derivative_uniformity) {
  ^^

<dawn>/test/tint/diagnostic_filtering/if_body_attribute.wgsl:6:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) @diagnostic(warning, derivative_uniformity) {
      ^

Texture2D<float4> t : register(t1);
SamplerState s : register(s2);

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  if ((x > 0.0f)) {
  }
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
