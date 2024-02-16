#version 310 es

struct mat4x2_f32 {
  vec2 col0;
  vec2 col1;
  vec2 col2;
  vec2 col3;
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  mat4x2_f32 inner[4];
} a;

layout(binding = 1, std430) buffer s_block_ssbo {
  float inner;
} s;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

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

vec2 load_a_inner_p0_p1(uint p0, uint p1) {
  switch(p1) {
    case 0u: {
      return a.inner[p0].col0;
      break;
    }
    case 1u: {
      return a.inner[p0].col1;
      break;
    }
    case 2u: {
      return a.inner[p0].col2;
      break;
    }
    case 3u: {
      return a.inner[p0].col3;
      break;
    }
    default: {
      return vec2(0.0f);
      break;
    }
  }
}

float load_a_inner_p0_p1_0(uint p0, uint p1) {
  switch(p1) {
    case 0u: {
      return a.inner[p0].col0[0u];
      break;
    }
    case 1u: {
      return a.inner[p0].col1[0u];
      break;
    }
    case 2u: {
      return a.inner[p0].col2[0u];
      break;
    }
    case 3u: {
      return a.inner[p0].col3[0u];
      break;
    }
    default: {
      return 0.0f;
      break;
    }
  }
}

void f() {
  mat4x2 p_a[4] = conv_arr4_mat4x2_f32(a.inner);
  int tint_symbol = i();
  mat4x2 p_a_i = conv_mat4x2_f32(a.inner[tint_symbol]);
  int tint_symbol_1 = i();
  vec2 p_a_i_i = load_a_inner_p0_p1(uint(tint_symbol), uint(tint_symbol_1));
  mat4x2 l_a[4] = conv_arr4_mat4x2_f32(a.inner);
  mat4x2 l_a_i = conv_mat4x2_f32(a.inner[tint_symbol]);
  vec2 l_a_i_i = load_a_inner_p0_p1(uint(tint_symbol), uint(tint_symbol_1));
  s.inner = (((load_a_inner_p0_p1_0(uint(tint_symbol), uint(tint_symbol_1)) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
