diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 warning: 'textureSample' must only be called from uniform control flow
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

diagnostic_filtering/while_loop_body_attribute.wgsl:7:3 note: control flow depends on possibly non-uniform value
  while (x > v.x) @diagnostic(warning, derivative_uniformity) {
  ^^^^^

diagnostic_filtering/while_loop_body_attribute.wgsl:8:9 note: return value of 'textureSample' may be non-uniform
    v = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Texture2D<float4> t : register(t1);
SamplerState s : register(s2);

struct tint_symbol_1 {
  float x : TEXCOORD0;
};

void main_inner(float x) {
  float4 v = (0.0f).xxxx;
  while((x > v.x)) {
    v = t.Sample(s, (0.0f).xx);
  }
}

void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.x);
  return;
}
