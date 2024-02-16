#version 310 es

struct mat4x2_f32 {
  vec2 col0;
  vec2 col1;
  vec2 col2;
  vec2 col3;
};

shared mat4x2 w[4];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      w[i] = mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f));
    }
  }
  barrier();
}

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat4x2_f32 inner[4];
} u;

mat4x2 conv_mat4x2_f32(mat4x2_f32 val) {
  return mat4x2(val.col0, val.col1, val.col2, val.col3);
}

mat4x2[4] conv_arr4_mat4x2_f32(mat4x2_f32 val[4]) {
  mat4x2 arr[4] = mat4x2[4](mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x2_f32(val[i]);
    }
  }
  return arr;
}

void f(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  w = conv_arr4_mat4x2_f32(u.inner);
  w[1] = conv_mat4x2_f32(u.inner[2u]);
  w[1][0] = u.inner[0u].col1.yx;
  w[1][0].x = u.inner[0u].col1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}
