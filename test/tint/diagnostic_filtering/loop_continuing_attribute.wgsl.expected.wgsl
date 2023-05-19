diagnostic_filtering/loop_continuing_attribute.wgsl:5:11 warning: 'dpdx' must only be called from uniform control flow
      _ = dpdx(1.0);
          ^^^^^^^^^

diagnostic_filtering/loop_continuing_attribute.wgsl:6:7 note: control flow depends on possibly non-uniform value
      break if x > 0.0;
      ^^^^^

diagnostic_filtering/loop_continuing_attribute.wgsl:6:16 note: user-defined input 'x' of 'main' may be non-uniform
      break if x > 0.0;
               ^

@fragment
fn main(@location(0) x : f32) {
  loop {

    continuing @diagnostic(warning, derivative_uniformity) {
      _ = dpdx(1.0);
      break if (x > 0.0);
    }
  }
}
