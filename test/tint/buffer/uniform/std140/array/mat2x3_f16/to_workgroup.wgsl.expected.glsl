#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x3_f16 {
  f16vec3 col0;
  f16vec3 col1;
};

shared f16mat2x3 w[4];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      w[i] = f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf));
    }
  }
  barrier();
}

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x3_f16 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float16_t inner;
} s;

f16mat2x3 conv_mat2x3_f16(mat2x3_f16 val) {
  return f16mat2x3(val.col0, val.col1);
}

f16mat2x3[4] conv_arr4_mat2x3_f16(mat2x3_f16 val[4]) {
  f16mat2x3 arr[4] = f16mat2x3[4](f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x3(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat2x3_f16(val[i]);
    }
  }
  return arr;
}

void f(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  w = conv_arr4_mat2x3_f16(u.inner);
  w[1] = conv_mat2x3_f16(u.inner[2u]);
  w[1][0] = u.inner[0u].col1.zxy;
  w[1][0].x = u.inner[0u].col1[0u];
  s.inner = w[1][0].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(gl_LocalInvocationIndex);
  return;
}
