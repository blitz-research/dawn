void set_vector_element(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_1(inout int2 vec, int idx, int val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_2(inout uint2 vec, int idx, uint val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_3(inout bool2 vec, int idx, bool val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_vector_element_4(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_5(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_vector_element_6(inout int3 vec, int idx, int val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_7(inout int4 vec, int idx, int val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_vector_element_8(inout uint3 vec, int idx, uint val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_9(inout uint4 vec, int idx, uint val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_vector_element_10(inout bool3 vec, int idx, bool val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_vector_element_11(inout bool4 vec, int idx, bool val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

[numthreads(1, 1, 1)]
void main() {
  float2 v2f = float2(0.0f, 0.0f);
  float3 v3f = float3(0.0f, 0.0f, 0.0f);
  float4 v4f = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int2 v2i = int2(0, 0);
  int3 v3i = int3(0, 0, 0);
  int4 v4i = int4(0, 0, 0, 0);
  uint2 v2u = uint2(0u, 0u);
  uint3 v3u = uint3(0u, 0u, 0u);
  uint4 v4u = uint4(0u, 0u, 0u, 0u);
  bool2 v2b = bool2(false, false);
  bool3 v3b = bool3(false, false, false);
  bool4 v4b = bool4(false, false, false, false);
  {
    for(int i = 0; (i < 2); i = (i + 1)) {
      set_vector_element(v2f, i, 1.0f);
      set_vector_element_1(v2i, i, 1);
      set_vector_element_2(v2u, i, 1u);
      set_vector_element_3(v2b, i, true);
    }
  }
  int i = 0;
  set_vector_element_4(v3f, i, 1.0f);
  set_vector_element_5(v4f, i, 1.0f);
  set_vector_element_6(v3i, i, 1);
  set_vector_element_7(v4i, i, 1);
  set_vector_element_8(v3u, i, 1u);
  set_vector_element_9(v4u, i, 1u);
  set_vector_element_10(v3b, i, true);
  set_vector_element_11(v4b, i, true);
  return;
}
