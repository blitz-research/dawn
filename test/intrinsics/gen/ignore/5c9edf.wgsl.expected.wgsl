[[group(1), binding(0)]] var arg_0 : external_texture;

fn ignore_5c9edf() {
  ignore(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  ignore_5c9edf();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  ignore_5c9edf();
}

[[stage(compute)]]
fn compute_main() {
  ignore_5c9edf();
}
