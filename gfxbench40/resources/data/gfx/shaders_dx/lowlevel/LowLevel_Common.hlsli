struct VertexShaderInput
{
	float3 position : POSITION;
    float2 texCoord0  : TEXCOORD0;
	float4 color0 : COLOR0;
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
    float2 texCoord0  : TEXCOORD0;
	float4 color0 : COLOR0;
};

struct FillPixelShaderInput
{
	float4 position : SV_POSITION;
    float2 texCoord0  : TEXCOORD0;
    float2 texCoord1  : TEXCOORD1;
    float2 texCoord2  : TEXCOORD2;
    float2 texCoord3  : TEXCOORD3;
};