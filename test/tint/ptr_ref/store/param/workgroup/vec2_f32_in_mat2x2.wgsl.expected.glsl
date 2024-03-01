#version 310 es

shared mat2 S;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    S = mat2(vec2(0.0f), vec2(0.0f));
  }
  barrier();
}

void func_S_X(uint pointer[1]) {
  S[pointer[0]] = vec2(0.0f);
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint tint_symbol_1[1] = uint[1](1u);
  func_S_X(tint_symbol_1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
