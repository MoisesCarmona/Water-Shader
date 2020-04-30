

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
textureCUBE skyMap : register(t0); // first texture

SamplerState filters: register(s0); // filter 0 using CLAMP, filter 1 using WRAP





float4 main(float4 WVPposition : SV_POSITION, float2 baseUV : TEXCOORD, float3 nrm : NORMAL, float4 modulate : OCOLOR, float4 RawPosition : POSITION, float time : TIME) : SV_TARGET
{
	//Per-Pixel Spotlight

float4 baseColor = skyMap.Sample(filters, RawPosition); // get base color


return baseColor; // return a transition based on the detail alpha

}