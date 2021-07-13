struct tint_symbol_1 {
  float2 a_particlePos : TEXCOORD0;
  float2 a_particleVel : TEXCOORD1;
  float2 a_pos : TEXCOORD2;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

tint_symbol_2 vert_main(tint_symbol_1 tint_symbol) {
  const float2 a_particlePos = tint_symbol.a_particlePos;
  const float2 a_particleVel = tint_symbol.a_particleVel;
  const float2 a_pos = tint_symbol.a_pos;
  float angle = -(atan2(a_particleVel.x, a_particleVel.y));
  float2 pos = float2(((a_pos.x * cos(angle)) - (a_pos.y * sin(angle))), ((a_pos.x * sin(angle)) + (a_pos.y * cos(angle))));
  const tint_symbol_2 tint_symbol_9 = {float4((pos + a_particlePos), 0.0f, 1.0f)};
  return tint_symbol_9;
}

struct tint_symbol_3 {
  float4 value : SV_Target0;
};

tint_symbol_3 frag_main() {
  const tint_symbol_3 tint_symbol_10 = {float4(1.0f, 1.0f, 1.0f, 1.0f)};
  return tint_symbol_10;
}

cbuffer cbuffer_params : register(b0, space0) {
  uint4 params[2];
};
RWByteAddressBuffer particlesA : register(u1, space0);
RWByteAddressBuffer particlesB : register(u2, space0);

struct tint_symbol_5 {
  uint3 gl_GlobalInvocationID : SV_DispatchThreadID;
};

[numthreads(1, 1, 1)]
void comp_main(tint_symbol_5 tint_symbol_4) {
  const uint3 gl_GlobalInvocationID = tint_symbol_4.gl_GlobalInvocationID;
  uint index = gl_GlobalInvocationID.x;
  if ((index >= 5u)) {
    return;
  }
  float2 vPos = asfloat(particlesA.Load2((16u * index)));
  float2 vVel = asfloat(particlesA.Load2(((16u * index) + 8u)));
  float2 cMass = float2(0.0f, 0.0f);
  float2 cVel = float2(0.0f, 0.0f);
  float2 colVel = float2(0.0f, 0.0f);
  int cMassCount = 0;
  int cVelCount = 0;
  float2 pos = float2(0.0f, 0.0f);
  float2 vel = float2(0.0f, 0.0f);
  {
    for(uint i = 0u; (i < 5u); i = (i + 1u)) {
      if ((i == index)) {
        continue;
      }
      pos = asfloat(particlesA.Load2((16u * i))).xy;
      vel = asfloat(particlesA.Load2(((16u * i) + 8u))).xy;
      if ((distance(pos, vPos) < asfloat(params[0].y))) {
        cMass = (cMass + pos);
        cMassCount = (cMassCount + 1);
      }
      if ((distance(pos, vPos) < asfloat(params[0].z))) {
        colVel = (colVel - (pos - vPos));
      }
      if ((distance(pos, vPos) < asfloat(params[0].w))) {
        cVel = (cVel + vel);
        cVelCount = (cVelCount + 1);
      }
    }
  }
  if ((cMassCount > 0)) {
    cMass = ((cMass / float2(float(cMassCount), float(cMassCount))) - vPos);
  }
  if ((cVelCount > 0)) {
    cVel = (cVel / float2(float(cVelCount), float(cVelCount)));
  }
  vVel = (((vVel + (cMass * asfloat(params[1].x))) + (colVel * asfloat(params[1].y))) + (cVel * asfloat(params[1].z)));
  vVel = (normalize(vVel) * clamp(length(vVel), 0.0f, 0.100000001f));
  vPos = (vPos + (vVel * asfloat(params[0].x)));
  if ((vPos.x < -1.0f)) {
    vPos.x = 1.0f;
  }
  if ((vPos.x > 1.0f)) {
    vPos.x = -1.0f;
  }
  if ((vPos.y < -1.0f)) {
    vPos.y = 1.0f;
  }
  if ((vPos.y > 1.0f)) {
    vPos.y = -1.0f;
  }
  particlesB.Store2((16u * index), asuint(vPos));
  particlesB.Store2(((16u * index) + 8u), asuint(vVel));
  return;
}
