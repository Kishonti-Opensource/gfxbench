cbuffer ShaderConstantBufferFilter : register(b3)
{
	float2		screen_resolution;
	float2		inv_screen_resolution;

	float4		depth_parameters;

	float2		offset_2d;
	float		dof_strength;
	float		camera_focus;
};
