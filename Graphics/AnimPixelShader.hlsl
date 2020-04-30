
texture2D diffuseTexture : register(t0);
texture2D emissiveTexture : register(t1);
texture2D specularTexture : register(t2);


SamplerState filters: register(s0); // filter 0 using CLAMP, filter 1 using WRAP

cbuffer SHADER_VARS : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 CameraPosition;

}




float4 main(float4 WVPposition : SV_POSITION, float2 baseUV : TEXCOORD, float3 nrm : NORMAL, float4 modulate : OCOLOR, float4 RawPosition : POSITION, float time : TIME) : SV_TARGET
{


	//Point Light

	float3 pointLightPOS = { 0.0f,300.0f, 300.0f };





	float4 SurfacePOS = RawPosition;
	float4 pointLightColor = { 1,1,1,1 };
	float3 LightDir = normalize(pointLightPOS - SurfacePOS);

	float Attenuation = 0;

	Attenuation = 1.0f - saturate(length(pointLightPOS - SurfacePOS) / 20.0f);
	float LightRatio = saturate(dot(LightDir, nrm)) * Attenuation;
	float4 PLight = LightRatio * pointLightColor;

	//Specular

	float Light2Ratio;



	float3 WorldPos = mul(RawPosition, worldMatrix);
	float3 Eye = CameraPosition.xyz;

	float3 viewDir = normalize(Eye - WorldPos);
	float3 Reflect = reflect(normalize(-normalize(pointLightPOS)), normalize(nrm));
	float4 specular = 20.0f * specularTexture.Sample(filters,baseUV) * pow(saturate(dot(Reflect, viewDir)), 100.0f);


	float4 baseColor = (diffuseTexture.Sample(filters, baseUV) + emissiveTexture.Sample(filters, baseUV) ) + (specular + PLight); // get base color


return baseColor; // return a transition based on the detail alpha

}