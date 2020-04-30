
struct OutputVertex
{
	float4 position : SV_POSITION; // System Value
	float4 color : OCOLOR;
};

float4 main(OutputVertex inputPixel) : SV_TARGET
{

	return inputPixel.color;

}