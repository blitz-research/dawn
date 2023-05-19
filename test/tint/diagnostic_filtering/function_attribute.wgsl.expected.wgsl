diagnostic_filtering/function_attribute.wgsl:7:9 warning: 'textureSample' must only be called from uniform control flow
    _ = textureSample(t, s, vec2(0, 0));
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

diagnostic_filtering/function_attribute.wgsl:6:3 note: control flow depends on possibly non-uniform value
  if (x > 0) {
  ^^

diagnostic_filtering/function_attribute.wgsl:6:7 note: user-defined input 'x' of 'main' may be non-uniform
  if (x > 0) {
      ^

@group(0) @binding(1) var t : texture_2d<f32>;

@group(0) @binding(2) var s : sampler;

@fragment @diagnostic(warning, derivative_uniformity)
fn main(@location(0) x : f32) {
  if ((x > 0)) {
    _ = textureSample(t, s, vec2(0, 0));
  }
}
