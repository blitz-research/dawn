[[group(1), binding(0)]] var arg_0 : texture_depth_2d_array;

fn ignore_c8a0ee() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_c8a0ee();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_c8a0ee();
}

[[stage(compute)]]
fn compute_main() {
  ignore_c8a0ee();
}
