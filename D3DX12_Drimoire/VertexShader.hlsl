struct VSInput
{
	float4 Position: POSITION;
	float2 Uv: TEXCOORD;
};

struct VSOutput
{
	float4 Position: SV_POSITION;
	float2 Uv: TEXCOORD;
};

VSOutput main( VSInput In )
{
	VSOutput result;
	result.Position = In.Position;
	result.Uv = In.Uv;
	return result;
}