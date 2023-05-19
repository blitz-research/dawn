diagnostic_filtering/switch_statement_attribute.wgsl:7:27 warning: 'dpdx' must only be called from uniform control flow
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                          ^^^^^^^^^

diagnostic_filtering/switch_statement_attribute.wgsl:7:24 note: control flow depends on possibly non-uniform value
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
                       ^^

diagnostic_filtering/switch_statement_attribute.wgsl:7:15 note: user-defined input 'x' of 'main' may be non-uniform
  switch (i32(x == 0.0 && dpdx(1.0) == 0.0)) {
              ^

@group(0) @binding(1) var t : texture_2d<f32>;

@group(0) @binding(2) var s : sampler;

@fragment
fn main(@location(0) x : f32) {
  @diagnostic(warning, derivative_uniformity) switch(i32(((x == 0.0) && (dpdx(1.0) == 0.0)))) {
    default: {
    }
  }
}
