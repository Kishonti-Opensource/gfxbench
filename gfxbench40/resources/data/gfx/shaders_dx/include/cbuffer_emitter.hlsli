cbuffer ShaderConstantBufferEmitter : register(b4)
{
	float4x4 emitter_worldmat;
	float4x4 emitter_mv;
	float4x4 emitter_mvp;

	uint	emitter_startBirthIdx;
	uint	emitter_endBirthIdx;
	float	emitter_begin_size;
	float	emitter_end_size;

	float3	emitter_aperture;
	float	emitter_focusdist;

	float3	emitter_min_freq;
	float	emitter_min_speed;

	float3	emitter_max_freq;
	float	emitter_max_speed;

	float3	emitter_min_amplitude;
	float	emitter_min_accel;

	float3	emitter_max_amplitude;
	float	emitter_max_accel;

	float4	emitter_color;

	float3	emitter_externalVelocity;
	float	emitter_gravityFactor;

	float	emitter_maxlife;
};
