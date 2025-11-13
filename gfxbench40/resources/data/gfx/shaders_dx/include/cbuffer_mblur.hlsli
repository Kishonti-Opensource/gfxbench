cbuffer ConstantBufferMBlur : register(b2)
{
    float4x4	mvp;
    float4x4	mvp2;
	float4x3	bones[32];
	float4x3	prev_bones[32];
};
