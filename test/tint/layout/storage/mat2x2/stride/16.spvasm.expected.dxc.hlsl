struct strided_arr {
  float2 el;
};

RWByteAddressBuffer ssbo : register(u0);

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

typedef strided_arr mat2x2_stride_16_to_arr_ret[2];
mat2x2_stride_16_to_arr_ret mat2x2_stride_16_to_arr(float2x2 m) {
  const strided_arr tint_symbol = {m[0u]};
  const strided_arr tint_symbol_1 = {m[1u]};
  const strided_arr tint_symbol_2[2] = {tint_symbol, tint_symbol_1};
  return tint_symbol_2;
}

strided_arr ssbo_load_1(uint offset) {
  const strided_arr tint_symbol_3 = {asfloat(ssbo.Load2((offset + 0u)))};
  return tint_symbol_3;
}

typedef strided_arr ssbo_load_ret[2];
ssbo_load_ret ssbo_load(uint offset) {
  strided_arr arr_1[2] = (strided_arr[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = ssbo_load_1((offset + (i * 16u)));
    }
  }
  return arr_1;
}

void ssbo_store_1(uint offset, strided_arr value) {
  ssbo.Store2((offset + 0u), asuint(value.el));
}

void ssbo_store(uint offset, strided_arr value[2]) {
  strided_arr array_1[2] = value;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      ssbo_store_1((offset + (i_1 * 16u)), array_1[i_1]);
    }
  }
}

void f_1() {
  const float2x2 x_15 = arr_to_mat2x2_stride_16(ssbo_load(0u));
  ssbo_store(0u, mat2x2_stride_16_to_arr(x_15));
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
