Texture2D shaderTexture : register(t0);
SamplerState filters : register (s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
	float4 diffuseColor;
	float3 lightDirection;
};

struct PixelOutputVertex
{
	float4 position: SV_POSITION;
	float2 uv :TEXCOORD0;
	float3 normal: NORMAL;
	float clip : ClipDistance;

};

float4 main(PixelOutputVertex input) : SV_TARGET
{
	float4 textureColor; 
	float3 lightDir;
	float lightIntensity;
	float4 color;

	//Sample the textue pixel at this location.
	textureColor = shaderTexture.Sample(filters, input.uv);

	//Set the default output color to the ambient light value for all pixels.
	color = ambientColor;

	//Invert the light direction for calculations.
	lightDir = -lightDirection;

	//Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightDir));

	if (lightIntensity > 0.0f)
	{
		//Determine the final diffuse color based on the diffuse color and the amount of light intensity.
		color += (diffuseColor * lightIntensity);

	}

	//Saturate final light color.
	color = saturate(color);

	//Multiply the texture pixel and the input color to get the final result
	color = color * textureColor;

	return color;
	

}