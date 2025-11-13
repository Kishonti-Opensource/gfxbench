/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
_float mod(_float x, _float y)
{
	return fmod(x,y) ;
}

vertex ParticleInOut shader_main( VertexInput    vi [[ stage_in ]],
								 constant VertexUniforms *vu [[ buffer(VERTEX_UNIFORM_DATA_INDEX) ]],
								 constant ParticleData *pd   [[ buffer(PARTICLE_DATA_INDEX) ]],
								 constant ParticleData *pc   [[ buffer(PARTICLE_COLOR_INDEX) ]])
{
    ParticleInOut r ;
  
    _float2 in_texcoord0 = vi.in_texcoord0.xy ;  
    
    _float3 up; 
	_float3 right;
	_float3 fwd;

	up = _float3( vu->mv[0][1], vu->mv[1][1], vu->mv[2][1]);

	right = _float3( vu->mv[0][0], vu->mv[1][0], vu->mv[2][0]);
	
	int idx = int(vi.in_position.x);

	_float2 offset = in_texcoord0 * _float(2.0) - _float(1.0);
	
	offset *=  _float(0.2) + sqrt(pd->d[idx].w) * _float(1.8);

	_float3 position = pd->d[idx].xyz + offset.x * right + offset.y * up;

	r.out_position = vu->mvp * _float4( position, 1.0);

	_float deltay = floor( pc->d[idx].w * _float(63.0) );
	_float deltax = floor( mod( deltay, _float(8.0) )) ;
	
	deltay = floor( deltay / _float(8.0) );
	
	r.out_texcoord0 = v_float2( (deltax + in_texcoord0.x) / _float(8.0), (_float(1.0) + deltay - in_texcoord0.y) / _float(8.0) );
		
	r.out_life = v_float( pd->d[idx].w );
	
#if defined USE_STEAM
	r.out_color = v_float4( pc->d[idx] );
#endif

#if defined USE_SMOKE
	r.world_texcoord = v_float2( (vu->world_fit_matrix * _float4( position, 1.0)).xy );
#endif
    
    return r;
}

