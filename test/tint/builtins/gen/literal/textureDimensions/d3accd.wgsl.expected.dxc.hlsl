TextureCube arg_0 : register(t0, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureDimensions_d3accd() {
  uint3 tint_tmp;
  arg_0.GetDimensions(1u, tint_tmp.x, tint_tmp.y, tint_tmp.z);
  uint2 res = tint_tmp.xy;
  prevent_dce.Store2(0u, asuint(res));
}

struct tint_symbol {
  float4 value : SV_Position;
};

float4 vertex_main_inner() {
  textureDimensions_d3accd();
  return (0.0f).xxxx;
}

tint_symbol vertex_main() {
  float4 inner_result = vertex_main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}

void fragment_main() {
  textureDimensions_d3accd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureDimensions_d3accd();
  return;
}
