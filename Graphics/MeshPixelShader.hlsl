

//
//texture2D baseTexture : register( t0);
//texture2D baseTexture : register(t0);
//
//
//SamplerState filter : register (s0);
//
//
//
//float4 main(float2 baseUV:TEXCOORD0) : SV_TARGET
//{
//	return baseTexture.Sample(filter, baseUV);
//
//}
//
texture2D diffuseTexture : register(t0); // first texture


SamplerState filters: register(s0); // filter 0 using CLAMP, filter 1 using WRAP




float4 main(float4 WVPposition : SV_POSITION, float2 baseUV : TEXCOORD, float3 nrm : NORMAL, float4 modulate : OCOLOR, float4 RawPosition : POSITION, float time : TIME) : SV_TARGET
{
	//Per-Pixel Spotlight


float4 spotLightColor = { 0,0,1,1 };
float4 ConeDir = { -1,-1,0,1};

float ConeRatio = .1f;
float innerConeRatio = .3f;
float4 spotLightPOS = { 0,500,0, 1 };
float3 LightDir = normalize(spotLightPOS - RawPosition);
float SurfaceRatio = saturate(dot(-LightDir, ConeDir));
float spotFactor = (SurfaceRatio > ConeRatio) ? 1 : 0;
float sAttenuation = 1.0f - saturate((innerConeRatio - SurfaceRatio) / (innerConeRatio - ConeRatio));
float LightRatio = saturate(dot(LightDir, nrm)) * sAttenuation;

float4 SpotLight = spotFactor * LightRatio * spotLightColor;

float4 baseColor = diffuseTexture.Sample(filters, baseUV); // get base color


return baseColor; // return a transition based on the detail alpha

}