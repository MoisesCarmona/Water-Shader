#pragma pack_matrix(row_major)




cbuffer SHADER_VARS : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

cbuffer ClipPlaneBuffer : register(b1)
{
	float4 clipPlane;
};

struct InputVertex
{  
	float4 position : POSITION; 
	float3 uvw : TEXCOORD;
	float3 nrm : NORMAL;

};

struct PixelOutputVertex
{
	float4 position: SV_POSITION;
	float2 uv :TEXCOORD0;
	float3 normal: NORMAL;
	float clip :ClipDistance;

};
[clipplanes(clipPlane)]
PixelOutputVertex main(InputVertex input)
{
	PixelOutputVertex output = (PixelOutputVertex)0;

	//Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	//Calculate the position of the vertex against the world, view and projection matrices
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	//Store the texture coordinate 
	output.uv = input.uvw.xy;

	//Calculate the normal vector against the world matrix
	output.normal = mul(input.nrm, worldMatrix);

	//Normalize the normal vector
	output.normal = normalize(output.normal);

	//Set the clipping plane.
	output.clip = dot(mul(input.position, worldMatrix), clipPlane);
	
	return output;


}