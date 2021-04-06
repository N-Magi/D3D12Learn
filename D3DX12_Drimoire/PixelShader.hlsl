struct VSOutput
{
	float4 Position: SV_POSITION;
	float2 Uv:TEXCOORD;
};

Texture2D<float4> tex: register(t0);
SamplerState smp: register(s0);

float4 main(VSOutput In) : SV_TARGET
{
	return float4(tex.Sample(smp, In.Uv));
}