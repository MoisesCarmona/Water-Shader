//Rule of Three
//Three things Need to Match
//1. C++ Vertex Struct
//2. Input Layout
// 3. HLSL Vertex Struct

#pragma pack_matrix(row_major)
//Or swap order of multiply

struct InputVertex
{   //xyz
	//uvw
	//normals
	float4 position : POSITION;
	float3 uvw : TEXCOORD;
	float3 nrm : NORMAL;
	int4 indices : BLENDINDICES;
	float4 weights : BLENDWEIGHTS;


};
struct OutputVertex
{
	float4 WVPposition : SV_POSITION; // System Value
	float2 uv : TEXCOORD;
	float3 nrm : NORMAL;
	float4 color :OCOLOR;
	float4 RawPosition : POSITION;
	float time : TIME;
};

cbuffer SHADER_VARS : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4 CameraPosition;
	float4x4 AnimMatrix[30];
}



OutputVertex main(InputVertex input)
{
	OutputVertex output = (OutputVertex)0;
	output.WVPposition = input.position;

	output.uv = input.uvw.xy;
	output.nrm = input.nrm;
	float4 skinned_pos = { 0.0f, 0.0f, 0.0f, 0.0f };
	float4 skinned_norm = { 0.0f, 0.0f, 0.0f, 0.0f };

	for (int j = 0; j < 4; j++)
	{
		if(input.indices[j] != -1)
		{ 

			skinned_pos += mul(float4(input.position.xyz, 1), AnimMatrix[input.indices[j]]) * input.weights[j];
				skinned_norm += mul(float4(input.nrm.xyz, 0.0f), AnimMatrix[input.indices[j]]) * input.weights[j];

		}
	}

    output.WVPposition = skinned_pos;
	output.nrm = skinned_norm;
	// Do math here (shader intrinsics)
	output.RawPosition = input.position;
	output.WVPposition = mul(output.WVPposition, worldMatrix);
	output.WVPposition = mul(output.WVPposition, viewMatrix);
	output.WVPposition = mul(output.WVPposition, projectionMatrix);
	// dont do perspective divide



	return output;
}