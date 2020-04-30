

Texture2D normalTexture : register(t0);

Texture2D reflectionTexture  : register(t1);

Texture2D refractionTexture : register(t2);

SamplerState filters: register(s0);

cbuffer Waterbuffer : register(b0)
{
	float waterTranslation;
	float reflectRefractScale;
	float2 padding;
};

struct OutputPixelVertex
{
	float4 position: SV_POSITION;
	float2 uv: TEXCOORD0;
	float4 reflectionPosition: TEXCOORD1;
	float4 refractionPosition:TEXCOORD2;
	float3 viewDir : VIEW;


};

float4 main(OutputPixelVertex input) :SV_TARGET
{
	float2 reflectTexCoord;
	float2 refractTexCoord;
	float4 normalMap;
	float3 normal;
	float4 reflectionColor;
	float4 refractionColor;
	float4 color;

	input.uv.y += waterTranslation * 0.005f;

	//Calculate the projected reflection texture coordinates
	reflectTexCoord.x = input.reflectionPosition.x / input.reflectionPosition.w / 2.0f + 0.5f;
	reflectTexCoord.y = -input.reflectionPosition.y / input.reflectionPosition.w / 2.0f + 0.5f;

	//Calculate the projected refraction texture coordinates
	refractTexCoord.x = input.refractionPosition.x / input.refractionPosition.w / 2.0f + 0.5f;
	refractTexCoord.y = -input.refractionPosition.y / input.refractionPosition.w / 2.0f + 0.5f;

	
	//Sample the normal from the normalmap texture
	normalMap = normalTexture.Sample(filters, input.uv);

	//Expand the range of the normal from (0,1) to (-1 to 1)
	normal = (normalMap.xyz * 2.0f) - 1.0f;
	

	//Repostion the texture coordinate sampling position by the normal map value to simulate wave effect
	reflectTexCoord = reflectTexCoord + (normal.xy * reflectRefractScale);
	refractTexCoord = refractTexCoord + (normal.xy * reflectRefractScale);
	
	//Sample the texture pixels from the textures using the updated texture coordinates
	reflectionColor = reflectionTexture.Sample(filters, reflectTexCoord);
	refractionColor = refractionTexture.Sample(filters, refractTexCoord);

	float4 DirLightColor = { 1, 1, 1, 1 };

	float3 DirLight2POS = { 0.3f,0.0f ,0 };
	float3 Reflect = reflect(normalize(-normalize(DirLight2POS)), normalize(normal));
	float4 specular = 5.0f * DirLightColor * pow(saturate(dot(Reflect, input.viewDir)),100.0f);


	//Combine reflection and refraction results for the final color.
	color = lerp(reflectionColor, refractionColor, 0.6f) + specular;
	return color;





}