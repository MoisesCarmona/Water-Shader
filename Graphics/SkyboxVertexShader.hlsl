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


}



OutputVertex main(InputVertex input)
{
	OutputVertex output = (OutputVertex)0;
	output.WVPposition = float4(input.position.xyz, 1.0f);
	
	output.uv = input.uvw.xy;
	output.nrm = input.nrm;

		float LightRatio;
	//float3 DirLightPOS = { -0.5,-0.2,-0.5f }; //upper Right Behind
	float3 DirLightPOS = { -10,0,0 }; //lower left Behind

	DirLightPOS = mul(DirLightPOS, worldMatrix);
	//DirLightPOS.z += sin(DirLightPOS.z + (output.time) * 0.1f);
	//DirLightPOS.x += sin(DirLightPOS.x + (output.time) * 0.1f);
	//DirLightPOS.y += sin(DirLightPOS.y + (output.time) * 0.1f);


	float4 DirLightColor = { 1, 1, 1, 1 };


	LightRatio = saturate(dot(DirLightPOS, input.nrm));
	float4 lColorLightR = DirLightColor * LightRatio;
	float4 DRLight = lColorLightR;

	

	//Final Light
	output.color = DRLight;// +PLight;



	// Do math here (shader intrinsics)
	output.RawPosition = input.position;
	output.WVPposition = mul(output.WVPposition, worldMatrix);
	output.WVPposition = mul(output.WVPposition, viewMatrix);
	output.WVPposition = mul(output.WVPposition, projectionMatrix).xyww;
	
	// dont do perspective divide



	return output;
}