#pragma pack_matrix(row_major)




cbuffer SHADER_VARS : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 camPos;

}

cbuffer ReflectionBuffer: register(b1)
{

	float4x4 reflectionMartrix;
}
struct InputVertex
{
	float4 position : POSITION;
	float3 uvw : TEXCOORD;
	//float3 nrm : NORMAL;

};
struct OutputPixelVertex
{
	float4 position: SV_POSITION;
	float2 uv: TEXCOORD0;
	float4 reflectionPosition: TEXCOORD1;
	float4 refractionPosition:TEXCOORD2;
	float3 viewDir : VIEW;




};
OutputPixelVertex main(InputVertex input)
{
	OutputPixelVertex output = (OutputPixelVertex)0;
	float4x4 reflectProjectWorld;
	float4x4 viewProjectWorld;


	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	
	output.uv = input.uvw.xy;
	////Create the reflectionProjectionwWorldMatrix
	reflectProjectWorld = mul(reflectionMartrix, projectionMatrix);
	reflectProjectWorld = mul(worldMatrix, reflectProjectWorld);

	output.reflectionPosition = mul(input.position, reflectProjectWorld);

	////Create the view Projection world Matrix for refraction
	viewProjectWorld = mul(viewMatrix, projectionMatrix);
	viewProjectWorld = mul(worldMatrix, viewProjectWorld);

	output.refractionPosition = mul(input.position, viewProjectWorld);


	float3 WorldPos = mul(input.position, worldMatrix);
	 output.viewDir = normalize(camPos - WorldPos);


	return output;

}
