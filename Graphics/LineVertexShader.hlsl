//Rule of Three
//Three things Need to Match
//1. C++ Vertex Struct
//2. Input Layout
// 3. HLSL Vertex Struct

#pragma pack_matrix(row_major)
//Or swap order of multiply

struct InputVertex
{   //xyzw
	//rgba
	float4 position : POSITION;
	float4 color : COLOR;



};
struct OutputVertex
{
	float4 position : SV_POSITION; // System Value
	float4 color :OCOLOR;

};

cbuffer SHADER_VARS : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
}

//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
	//return pos;
//}

OutputVertex main(InputVertex input)
{
	OutputVertex output = (OutputVertex)0;
	output.position = input.position;
	output.color = input.color;
	// Do math here (shader intrinsics)

	output.position = mul(output.position, worldMatrix);
	//output.position.x += 2 * sin( 2* output.position.y + 0.5f * output.time);
	//output.position.y += 10 * sin(10 * output.position.x + 0.5f * output.time);
	//output.position.z += 2 * sin( 2* output.position.x + 0.5f* output.time);
	/*if (output.position.y > 0.5f)
	{
		output.color.z--;
		output.color.x++;
		output.color.y = 0;
	}
	else
	{
		output.color.z++;
		output.color.x--;
		output.color.y = 0;

	}*/
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	// dont do perspective divide



	return output;
}